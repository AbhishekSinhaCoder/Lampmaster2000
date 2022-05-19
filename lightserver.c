/*

This is the server program to control the power switch.  It will be 
activated by any TCP/IP client program with the appropriate commands,
"ON" and "OFF".  Several stages are designed for this process.

Stage 1:  Just get it working.  Have a client send the ON and OFF commands
and activate or deactivate the switch accordingly. : DONE

Stage 2:  Put in a timer to automatically turn off the switch after a 
certain specified time frame, probably 5 minutes, to prevent excessive
power consumption. : DONE

Stage 3:  Built a better circut.  Each parallel port can control 8 switches.
Actually, each port CAN support 12, but the more conventional method is to
use the 8 data ports.  Each switch requires a resistor, a transistor, a
diode, and a relay.  The part total for each switch is approximately $15.

Stage 4:  Build a circut which can apply variable voltage, controlled by
the parallel port.  This will be necessary for controlling a remote control
mechanism for an RC car, which is the next big project.


NOTE:  You have to be root to run this server, or otherwise have permissions
to directly access the parallel port.


This server has now been modified to operate a stepper motor.  It relies on
knowing the current state of the parallel port, and therefore is controlled
by the same process as the light is.  

Motor control is a bit more complicated.  The stepper motor in use was
obtained from a discarded floppy drive.  Each "step" turns the shaft 1.8
degrees.  The motor turns by applying voltage to different contacts on the
motor in a specific order, and the reverse order will turn the shaft in the
other direction.  

An additional constraint is that the motor must not be allowed to move beyond
upper and lower bounds.  Although the motor can handle infinite turns in any
direction, the camera mounted to it is connected by a cable and the rotation
bounds must be limited to prevent the cable from getting wrapped around the
shaft.  A 90-180 degree total field of vision should be adequate for any 
camera located on or near a wall.

If you're banging your head over how to successfully wire the stepper motor
to work properly with my code, you may wish to reorganize the code.  I have
the numbers out of sequence because I have the stepper motor contacts
wired out of sequence and I adjusted the program to match it.  When I 
move off of the breadboard and onto a more permanant PC board, I will 
rewire everything correctly and fix the program then.

To step the motor, it is important to know which contact was tapped previously,
unless a minimum of 7.2 degree turns is desired (in which case all 4 contacts
can be tapped in one command).

The motor, since it can be moved manually, should be calibrated each time
the server starts or any time the motor is moved inadvertantly.  A simple
calibration will be for the user to manually point the camera at the center
of the bounded region, then send a command to set the currentstep value
accordingly.  The calibration command can come from the client or the
server as an argument when it starts up.  Right now the program simply
assumes that the camera is positioned in the centre of the field of vision
and sets the currentstep value at the midpoint.


This program has now been modified to operate a remote control car.  I 
accomplished this by wiring relays to the remote control unit of the car.
These relays are controlled the same way the lamp relay is.  This project
consumed an additional 4 transistors.

Please send any relavent comments, suggestions, complaints, but no spam to
restil@alignment.net.  This code works for me.  I claim nothing more.  
Especially if you are somewhat new to interfacing electronics with the 
computer, try to use a computer that you can afford to damage.  I use a 486/66
that I got for $30.  Its adaquate for this project, and with networking, 
I don't have to put my primary servers in any danger.

You are free to modify, redistribute, sell, whatever this code.  If you make
useful changes, feel free to update me with those changes, I'd appreciate
it.


Now modded to include commands to operate the X10 stuff.  This program will
keep track of the time limits, as well has have different time limits for
those devices, AND the lamp.  Now we get 3 minutes for teh internet peeps and
longer durations for local selections.


New remake of this program:

Use configuration files for all devices.

New multithreaded environment for handling longer lasting actions.

Return status data from all commands, saving a second call for status data.

Redirect option to send a signal to another server for other housecodes.


*/





#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/io.h>

/*
all this data will be definied in config files, although defaults remain
*/
#define base 0x378
#define base2 0x278
#define VERYSLOW 100000
#define SLOW 200000
#define MEDIUM 200000
#define FAST 300000

#define DATA "You got it from me"


child_handler(int arg)
{
int     status;
wait(&status);
}        

pipecleaner(int arg)
{
}

struct	waitqueue
	{
	char	host[20];
	int	priority;	//0 is lowest
	long	starttime;	// time entered the queue
	long	lastupdate;	// time of the last heartbeat check
	struct	waitqueue	*next;
	};

struct	rccareventqueue
	{
	char	host[20];
	int	type;	//0 - power, 1 - turn
	int	direction; // f/b, or 0,1,2 (left, right, center)
	int	speed;
	struct	rccareventqueue *next;
	};

struct	x10eventqueue
	{
	char	host[20];
	char	buf[256];
	struct	x10eventqueue *next;
	};


struct	switchdata
	{
	char	name[20];
	int	port;
	int	status;
	int	switchbase;
	int	control;
	int	counter;
	int	watts;		// watts used by this appliance.
	int	stateposition;	// position in the state variable
// timechecks
	int	timeout;
	int	startstatus;	// State the device is at midnight
	long	times[5];	// toggle availability at these times
	int	maxsecs;	// maximum minutes per day
	int	timeused;	// seconds used so far today.
	int	lock;		// is this locked?
	char	lockhost[20];	// Who holds the lock?
	long	lockexp;	// When does the lock expire?
	char	relayhost[30];	// relay host
	int	relayport;	// relay port
	struct	waitqueue *wq;
	};

struct	sdlink
	{
	struct	switchdata	sd;
	struct	sdlink 		*next;
	};

struct motordata
	{
	char	name[20];
	int	sequence[4];
	int	numsequence;
	int	motorbase;
	int	currentstep;
	int	lastcontact;
	int	control;
	int	lock;		// is this locked?
	char	lockhost[20];	// Who holds the lock?
	long	lockexp;	// When does the lock expire?
	char	relayhost[30];	// relay host
	int	relayport;	// relay port
	struct	waitqueue *wq;
	};

struct	mdlink
	{
	struct	motordata	md;
	struct	mdlink 		*next;
	};

struct rccardata
	{
	char	name[20];
	int	commonport;
	int	forwardport;
	int	backwardport;
	int	leftport;
	int	rightport;
	int	timeout;
	int	carbase;
	int	lock;		// is this locked?
	char	lockhost[20];	// Who holds the lock?
	long	lockexp;	// When does the lock expire?
	long	heartbeat;
	int	nudgef; // time in microseconds to delay
	int	nudgeb;
	int	pushf;
	int	pushb;
	int	wstatus;
	long	wcounter;
	int	control;
	char	relayhost[30];	// relay host
	int	relayport;	// relay port
	struct	waitqueue *wq;
	struct	rccareventqueue *eq;
	};

struct	X10data
	{
	int	state;
	int	limit;
	int	counter;	
	int	timeused;
	int	remotetimeout;
	int	localtimeout;
	int	watts;
	int	stateposition;	// position in the state variable
	int	startstatus;	// State the device is at midnight
	int	type;		// 1 - br, 2 - ppower
	long	times[5];	// toggle availability at these times
	int	maxsecs;	// maximum minutes per day
	int	lock;		// is this locked?
	char	lockhost[20];	// Who holds the lock?
	long	lockexp;	// When does the lock expire?
	int	control;	// does this server control this module?
	char	relayhost[30];	// relay host
	int	relayport;	// relay port
	int	relayhouse;	// housecode to relay this module to.
	struct	waitqueue *wq;
	};


struct	bandata
	{
	char	host[20];
	long	banexp;		// When does this ban expire?
	struct	bandata *next;
	};	

struct	banwatch
	{
	char	host[20];
	int	count;		// number of hits
	long	timestamp;	// last time something happened
	};

int	pipes[2];
struct	sdlink	*switches;
struct	mdlink	*motors;
struct	X10data	X10mods[17][17];
struct	x10eventqueue *x10eq;
int	X10state[17];
int	X10limit[17];
int	X10counter[17];
int	steps[4];  // sequence for stepper motor data pins

int	listenport;
int	bancheck;	// do we record/check for bans?
int	togglemax;	// max number of toggles before a ban
int	banhours;	// number of hours to enact a ban for.
int	bandeduct;	// number of seconds between one point reductions.
float	kwhcost;	// power cost per kwh

long	lastbanupdate;

int	locks[5];	// Seconds that locks last for each tier;

unsigned char	state;
unsigned char	state2;

struct	banwatch	banwatchlist[200];
struct	bandata		*bans;


int	ifbanned(char *host)
{
struct	bandata	*temp;
temp = bans;
while(temp != NULL)
	{
	if(strcmp(temp->host,host) == 0)
		return(1);
	temp = temp->next;
	}
return(0);
}

int	bancheckfunc(struct banwatch *banwatchlist, char *host)
{
int	x,y,z;
long	timestamp;
int	tsmark;

if(ifbanned(host))
	return(1);
// check for an open space, otherwise get the oldest timestamp;
timestamp = time(NULL);
tsmark = 0;
for(z=0;z<200;z++)
	{
	if(strcmp(banwatchlist[z].host,host) == 0)
		{
		banwatchlist[z].count++;
		if(banwatchlist[z].count > togglemax)
			addban(host);
		}
	}
for(z=0;z<200;z++)
	{
	if(banwatchlist[z].host[0] == 0 || banwatchlist[z].count == 0)
		{
		strcpy(banwatchlist[z].host,host);
		banwatchlist[z].count=1;
		banwatchlist[z].timestamp = time(NULL);	
		return(0);
		}
	if(banwatchlist[z].timestamp < banwatchlist[tsmark].timestamp)
		tsmark = z;
	}
strcpy(banwatchlist[tsmark].host,host);
banwatchlist[z].count=1;
banwatchlist[z].timestamp = time(NULL);
return(0);
}

int	addban(char *host)
{
int	x,y,z;
long	timestamp;
int	tsmark;
struct	bandata	*holder;
struct	bandata	*temp;

holder = malloc(sizeof *holder);
strcpy(holder->host,host);
holder->banexp = time(NULL) + banhours * (60*60*24);
holder->next=NULL;

if(bans==NULL)
	{
	bans=holder;
	}
else
	{
	temp=bans;
	while(temp->next != NULL)
		temp = temp->next;
	temp->next = holder;
	}
}

deletebans()
{
struct	bandata	*holder;
struct	bandata	*temp;
int	x,y,z;
if(time(NULL) > lastbanupdate + bandeduct)
	{
	lastbanupdate = time(NULL);
	for(z=0;z<200;z++)
		{
		if(banwatchlist[z].count > 0)
			banwatchlist[z].count--;
		}
	}
while(bans != NULL)
	{
	holder = bans;
	if(holder->banexp < time(NULL))
		{
		bans = holder->next;
		free(holder);
		}
	else
		return(0);
	}
}



// This function will take the base value and return the pointer to the
// correct state variable
unsigned char *getstatevar(int	basevar)
{
if(basevar == base)
	return(&state);
if(basevar == base2)
	return(&state2);
}

int	rccardequeue(struct rccardata *rccar)
{
struct	rccareventqueue *temp;

temp = rccar->eq;
if(temp == NULL)
	{
	return(0);
	}
printf("Dequeueing car command: %d %d %d\n",temp->type, temp->speed, temp->direction);
rccar->eq = rccar->eq->next;
rccar->heartbeat = time(NULL);
if(temp->type == 0)
	power(temp->speed,temp->direction, rccar, getstatevar(rccar->carbase));
else if(temp->type == 1)
	turn(temp->direction, rccar, getstatevar(rccar->carbase));
else
	powerturn(temp->direction >> 2, temp->direction & 3, temp->speed, rccar);
free(temp);
}

int rccarenqueue(int type, int speed, int direction, char *host, struct	rccardata *rccar)
{
struct	rccareventqueue *temp;
struct	rccareventqueue *holder;
// First, check to see if the car is locked.  If so, add to the queue.
if(rccar->lock && strcmp(rccar->lockhost,host) != 0)
	{
	printf("Car is currently locked by .%s.  %s is being added to the waitqueue.\n",rccar->lockhost,host);
	rccarwaitenqueue(host,rccar);
	return(1);
	}
holder = malloc(sizeof *holder);
strcpy(holder->host,host);
holder->type = type;
holder->speed = speed;
holder->direction = direction;
holder->next = NULL;
temp = rccar->eq;
if(temp == NULL)
	rccar->eq = holder;
else
	{
	if(strcmp(host,temp->host) == 0 && strcmp(host,"") != 0)
		{
		free(holder);
		return(0);
		}
	while(temp->next != NULL)
		{
		if(strcmp(host,temp->host) == 0 && strcmp(host,"") != 0)
			{
			free(holder);
			return(0);
			}
		}
		temp = temp->next;
	temp->next = holder;
	}
return(0);
}


/* 
This function adds a specific host to a wait queue.  When the rccar is locked,
the waitqueue will hold a list of users waiting.  When a lock expires, the
next host on the queue receives the lock.  Feedback to the client will permit
them to start playing.
*/

int rccarwaitenqueue(char *host, struct	rccardata *rccar)
{
struct	waitqueue *temp;
struct	waitqueue *holder;

holder = malloc(sizeof *holder);
strcpy(holder->host,host);
holder->priority = 0;
holder->lastupdate = time(NULL);
time(&holder->starttime);
holder->next = NULL;
temp = rccar->wq;
if(temp == NULL)
	rccar->wq = holder;
else
	{
	// already on the queue?  They will just reassume previous position.
	if(strcmp(host,temp->host) == 0 && strcmp(host,"") != 0)
		{
		free(holder);
		return(1);
		}
	while(temp->next != NULL)
		{
		temp = temp->next;
		if(strcmp(host,temp->host) == 0 && strcmp(host,"") != 0)
			{
			free(holder);
			return(1);
			}
		}
	temp->next = holder;
	}
return(1);
}

/*
This function removes the first entry from the waitqueue and sets the lock
for the selected host.  The expiration time it set for 5 minutes after current
time.  Lock is set at the time of dequeue.
*/

int	rccarwaitdequeue(struct rccardata *rccar)
{
struct	waitqueue *temp;
int	lastupdate;

while(1)
	{
	temp = rccar->wq;
	if(temp == NULL)
		{
		rccar->lock = 0;
		return(0);
		}
	rccar->lock = 1;
	rccar->wq = rccar->wq->next;
	strcpy(rccar->lockhost,temp->host);
	rccar->lockexp = time(NULL) + 5*60;
	rccar->heartbeat = time(NULL);
	lastupdate = temp->lastupdate;
	free(temp);
	if(lastupdate > time(NULL) - 60)
		break;
	}
}


int	x10dequeue(int *insanity, int inslamp)
{
struct	x10eventqueue *temp;

//printf("Dequeuing x10.\n");
temp = x10eq;
if(temp == NULL)
	{
	return(0);
	}
x10eq = x10eq->next;
//printf("buffer: %s\n",temp->buf);
if(strncmp(temp->buf,"X10ON",5) == 0)
	{
	x10lampon(temp->buf,insanity,inslamp);
	}
if(strncmp(temp->buf,"X10OF",5) == 0)
	{
	x10lampoff(temp->buf);
	}
free(temp);
}

int x10enqueue(char *buf, char *host)
{
struct	x10eventqueue *temp;
struct	x10eventqueue *holder;
int	hostcount;	//how many entires does this host have in the queue?

//printf("Enquing x10\n");
/*
if(bancheck)
	bancheckfunc(banwatchlist, host);
*/
hostcount=0;
holder = malloc(sizeof *holder);
strcpy(holder->buf,buf);
strcpy(holder->host,host);
holder->next = NULL;
temp = x10eq;
if(temp == NULL)
	x10eq = holder;
else
	{
	if(strcmp(buf,temp->buf) == 0) // already in the queue?
		{
		free(holder);
		return(0);
		}
	while(temp->next != NULL)
		{
		if(strcmp(buf,temp->buf) == 0) // command already in the queue?
			{
			free(holder);
			return(0);
			}
		if(strcmp(temp->host,host) == 0) // host already in queue.
			hostcount++;
		if(hostcount > 3)
			{
			free(holder);
			return(0);
			}
		temp = temp->next;
		}
	temp->next = holder;
	}
//printf("done enquing.\n");
}


/* isavailable
This function takes the starting availability at midnight and a list of 
times in minutes from midnight.  Figure out if the device is currently 
available.  0 for a time value means its the last of the list.
*/

int	isavailable(int start, long *times)
{
int	mins;
int	x,y,z;
long	t;
struct	tm	*tim;

//First, get the time now and convert it into minutes from midnight.
time(&t);
tim = localtime(&t);
mins = tim->tm_hour*60 + tim->tm_min;

// now find out what the current availability is
x = start;
for(z=0;z<5;z++)
	{
	if(times[z] == 0)
		break;
	if(times[z] <= mins)
		x = !x;
	}

return(x);
}




//int lampon(int portnum, int *status, long *counter, int *state, int *insanity, int inslamp)
int lampon(struct switchdata *sd, int *state, int *insanity, int inslamp)
{
char	s[256];
int	x;

x = 1 << sd->port;
// First, check to see that the lamp is currently off.
if(sd->status == 1)
	return(-1); 
// Next, check to see if the lamp is currently available
if(!isavailable(sd->startstatus,sd->times))
	return(-1);

// Next, see if the lamp is used up yet
if(sd->timeused > sd->maxsecs)
	return(-1);

if(ioperm(sd->switchbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", sd->switchbase), exit(1);
else
	{
	sd->status=1;
	sd->counter = time(NULL);
	sprintf(s,"ON");
	*state = *state | x;
   	outb(*state, sd->switchbase);
	*insanity = *insanity + (inslamp*3);
	printf("insanity: %d\n",*insanity);
	}
}


x10lampon(char *buf, int *insanity, int inslamp)
{	
int unit;
char	house;
int	housenum;
int	timelimit;
time_t	lt;
struct	tm	*tim;
char	s[256];


printf("turning on X10 unit.\n");
/* Let there be LIGHT!!! */
house = buf[6];
housenum = house - 'A';
unit = atoi(&buf[7]);
lt = time(NULL);
tim = localtime(&lt);

// First, check to see that the lamp is currently off.
if(X10mods[housenum][unit].state == 1)
	return(-1); 

// Next, check to see if the lamp is currently available
if(!isavailable(X10mods[housenum][unit].startstatus,X10mods[housenum][unit].times))
	return(-1);

// Next, see if the lamp is used up yet
if(X10mods[housenum][unit].timeused > X10mods[housenum][unit].maxsecs)
	return(-1);

if(unit != 10 || tim->tm_hour >= 6) 
	{
	X10mods[housenum][unit].counter = time(NULL);
	timelimit=atoi(&buf[10]);
	printf("Timelimit: %d\n",timelimit);
	if(timelimit == -1)
		{
		if(X10mods[housenum][unit].state == 0)
			X10mods[housenum][unit].limit = 60;
		}
	else if(timelimit == -2)
		{
		X10mods[housenum][unit].limit = 1;
		}
	else
		{
		X10mods[housenum][unit].limit = timelimit;
		if(!X10mods[housenum][unit].state)
			{
			sprintf(s,"/home/pjm/br %c%d on",house,unit);
			system(s);
			}
		*insanity = (inslamp) + *insanity;
		printf("insanity: %d\n",insanity);
		}
	X10mods[housenum][unit].state = 1;
	}
}

x10lampon2(int house, int unit, int *insanity, int inslamp)
{
char	buf[256];
int	x,y,z;

sprintf(buf,"XXXXXX%c%d",house + 'A', unit);
x10lampon(buf,insanity,inslamp);
}

x10lampoff(char *buf)
{
int unit;
char	house;
int	housenum;
int timelimit;
char	s[256];

printf("turning off X10 unit.\n");
// turn it off
house = buf[6];
housenum = house - 'A';
unit = atoi(&buf[7]);
timelimit=atoi(&buf[10]);
X10mods[housenum][unit].limit = 0;
if(timelimit != -1 && X10mods[housenum][unit].state)
	{
	sprintf(s,"/home/pjm/br %c%d off",house,unit);
	X10mods[housenum][unit].state = 0;
	system(s);
	}
}

x10lampoff2(int house, int unit)
{
char	buf[256];
int	x,y,z;

sprintf(buf,"XXXXXX%c%d",house + 'A', unit);
x10lampoff(buf);
}


int lampoff(struct switchdata *sd, int *state)
{
char	s[256];
int	x;
long	temp;

x = 1 << sd->port;
// First, make sure it's actually on, or we do nothing.
if (sd->status == 0)
	return(-1);
if (ioperm(sd->switchbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", sd->switchbase), exit(1);
else
	{
	sd->status=0;
	sprintf(s,"OFF");
	*state = *state & (255-x);
   	outb(*state, sd->switchbase);
	
	// Increment the timeused variable
	temp = time(NULL) - sd->counter;
	sd->timeused = sd->timeused + temp;	

	}
}

/*  pan
This function will pan a specified motor one step to the left or right
depending on the value of direction.  1 = left, otherwise right.  
The override variable if 1 will not do bounds checking (for calibration)
*/

int	pan(int direction,struct motordata *md, int *state,int override)
{
int	hold;
int	x,y,z;

x=0;
printf("Current state: %d\n",*state);
for(z=0;z<md->numsequence;z++)
	{
	x = x + md->sequence[z];
	}
printf("X: %d\n",x);

printf("Override = %d, direction = %d, currentstep = %d\n",override,direction,md->currentstep);

if(!override && direction == 1 && md->currentstep < 0) // Left barrier
	return(0);
if(!override && direction == 0 && md->currentstep > 70) // Right barrier
	return(0);
printf("Still here.\n");
if (ioperm(md->motorbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", md->motorbase), exit(1);
else
	{
	hold = md->lastcontact;
	if(direction == 1) // Left
		{
		md->lastcontact++;
		if(md->lastcontact > 3)
			md->lastcontact=0;
		md->currentstep--;
		}
	else
		{
		md->lastcontact--;
		if(md->lastcontact < 0)
			md->lastcontact=3;
		md->currentstep++;
		}
	*state = *state & 255-x;
	*state = *state | md->sequence[md->lastcontact] | md->sequence[hold];
	printf("New state: %d\n",*state);
   	outb(*state, md->motorbase);
	}
}

forwardon(struct rccardata *rccar, int *state2)
{
printf("Forward on.\n");
if (ioperm(rccar->carbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", rccar->carbase), exit(1);
else
	{
	*state2 = *state2 | rccar->forwardport;
	if(!rccar->wstatus)
		*state2 = *state2 | rccar->commonport;
	printf("State2 at TOF is: %d.\n",*state2);
	outb(*state2, rccar->carbase);
	}
}

forwardoff(struct rccardata *rccar, int *state2)
{
printf("Forward off.\n");
if (ioperm(rccar->carbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", rccar->carbase), exit(1);
else 
	{
	*state2 = *state2 & (255 - rccar->forwardport);
	if(!rccar->wstatus)
		*state2 = *state2 & (255 - rccar->commonport);
	printf("State2 at TOF is: %d.\n",*state2);
	outb(*state2, rccar->carbase);
	}
}

backwardon(struct rccardata *rccar, int *state2)
{
printf("Backward on.\n");
if (ioperm(rccar->carbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", rccar->carbase), exit(1);
else
	{
	*state2 = *state2 | rccar->backwardport;
	if(!rccar->wstatus)
		*state2 = *state2 | rccar->commonport;
	printf("State2 at TOB is: %d.\n",*state2);
	outb(*state2, rccar->carbase);
	}
}

backwardoff(struct rccardata *rccar, int *state2)
{
printf("Backward off.\n");
if (ioperm(rccar->carbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", rccar->carbase), exit(1);
else
	{
	*state2 = *state2 & (255 - rccar->backwardport);
	if(!rccar->wstatus)
		*state2 = *state2 & (255 - rccar->commonport);
	printf("State2 at TOB is: %d.\n",*state2);
	outb(*state2, rccar->carbase);
	}
}

power(int speed, int direction, struct rccardata *rccar, int *state2)
{
printf("Power function.  Direction = %d.  speed = %d.\n",direction, speed);
if(direction == 1)
	{
	backwardoff(rccar, state2);
	forwardon(rccar,state2);
	if(speed == 0)
		usleep(rccar->nudgef);
	else
		usleep(rccar->pushf);
	forwardoff(rccar,state2);
	sleep(1); 
	}
else
	{
	forwardoff(rccar,state2);
	backwardon(rccar, state2);
	if(speed == 0)
		usleep(rccar->nudgeb);
	else
		usleep(rccar->pushb);
	backwardoff(rccar,state2);
	sleep(1); 
	}
}


turnonleft(struct rccardata *rccar, int *state2)
{
if (ioperm(rccar->carbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", rccar->carbase), exit(1);
else
	{
	/* Turn on left */
	*state2 = *state2 | rccar->leftport | rccar->commonport;
   	outb(*state2,rccar->carbase);
	}
}

turnoffleft(struct rccardata *rccar, int *state2)
{
if (ioperm(rccar->carbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", rccar->carbase), exit(1);
else
	{
	/* Turn on left */
	*state2 = *state2 & (255 - (rccar->leftport + rccar->commonport));
   	outb(*state2,rccar->carbase);
	}
}

turnonright(struct rccardata *rccar, int *state2)
{
if (ioperm(rccar->carbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", rccar->carbase), exit(1);
else
	{
	*state2 = *state2 | rccar->rightport | rccar->commonport;
   	outb(*state2,rccar->carbase);
	}
}

turnoffright(struct rccardata *rccar, int *state2)
{
if (ioperm(rccar->carbase,1,1))
	fprintf(stderr, "Couldn't get the port at %x\n", rccar->carbase), exit(1);
else
	{
	/* Turn on left */
	*state2 = *state2 & (255 - (rccar->rightport + rccar->commonport));
   	outb(*state2,rccar->carbase);
	}
}


int	getdirection(struct rccardata *rccar, int *state2)
{
if(*state2 & rccar->rightport)
	return(2);
if(*state2 & rccar->leftport)
	return(1);
return(0);
}

turn(int direction, struct rccardata *rccar, int *state2)
{
rccar->wcounter = time(NULL);
if(direction == 1) // left
	{
	rccar->wstatus = 1;
	turnoffright(rccar,state2);
	turnonleft(rccar,state2);
	}
else if(direction == 2) // right
	{
	rccar->wstatus = 1;
	turnoffleft(rccar,state2);
	turnonright(rccar,state2);
	}
else // center
	{
	rccar->wstatus = 0;
	turnoffleft(rccar,state2);
	turnoffright(rccar,state2);
	}
}


int	powerturn(int	powerdirection, int movedirection, int speed, struct rccardata *rccar)
{
int	savedirection;

savedirection = getdirection(rccar, getstatevar(rccar->carbase));
turn(movedirection, rccar, getstatevar(rccar->carbase));
usleep(500000);
power(speed, powerdirection, rccar, getstatevar(rccar->carbase));
turn(savedirection, rccar, getstatevar(rccar->carbase));


}

int	sockinit(int port)
{
struct	sockaddr_in server;
struct	sockaddr_in client;
int	sock;
int	length;

sock = socket(AF_INET, SOCK_STREAM, 0);
printf("sock value is: %d\n",sock);
if(sock < 0)
	{
	perror("Opening steram socket");
	exit(1);
	}
server.sin_family = AF_INET;
server.sin_addr.s_addr = INADDR_ANY;
server.sin_port = htons((u_short )port);
if (bind(sock, &server, sizeof(server)))
	{
	perror("binding stream socket");
	exit(1);
	}
length = sizeof (server);
if(getsockname(sock, &server, &length))
	{
	perror("getting socket name");
	exit(1);
	}
printf("Socket has port #%d\n",ntohs(server.sin_port));
listen(sock, 5);
return(sock);
}


int	getconnection(int sock)
{
int	rval;
struct	sockaddr_in client;
fd_set	rfds;
struct	timeval	tv;
int	retval;
int	size;
int	msgsock;

size=sizeof (client);
/* Set up select to check the socket for data.  If none is available, 
   continue on and perform other tasks.  */	

FD_ZERO(&rfds);
FD_SET(sock,&rfds);
/* Wait 1 second for data, then proceed with other stuff */
	
tv.tv_sec=1;
tv.tv_usec=0;	
retval = select(11, &rfds, NULL, NULL, &tv);
if(retval)		/* we got data */
	{
	msgsock = accept(sock, &client,&size);
	if(msgsock == -1)
		{
		perror("accept");
		return(-1);
		}
	else 
		{
//		address_holder = (unsigned char *) &client.sin_addr.s_addr;	
	//	bzero(buf, sizeof(buf));

		printf("About to read.\n");
		FD_ZERO(&rfds);
		FD_SET(msgsock,&rfds);
/* Wait 1 second for data, then proceed with other stuff */
	
		tv.tv_sec=1;
		tv.tv_usec=0;	
		retval = select(11, &rfds, NULL, NULL, &tv);
		if(retval <= 0)		/* we got data */
			{
			close(msgsock);
			return(-1);
			rval=0;
			}
		else
			return(msgsock);
		}
	}
else
	return(-1);
}

int	getcommand(int msgsock, char *buf, char *name, char *command, char *value, char *host)
{
int	rval;
int	x,y,z;
for(z=0;z<1024;z++)
	buf[z] = 0;
if((rval = read(msgsock, buf, 1024)) < 0)
	{
	perror("Reading stream message");
	close(msgsock);
	return(-1);
	}

y = gettoken(0,buf,name);
y = gettoken(y,buf,command);
y = gettoken(y,buf,value);
y = gettoken(y,buf,host);
printf("rval is %d.\n",rval);
printf("buf is: %s.\n",buf);
}

int	gettoken(int pos, char *line, char *token)
{
int	x,y,z;
int	size;

z=pos;
if(line[z] != 0)
	{
	while(line[z] == ' ')
		z++;
	x = z++;
	while(line[z] != ' ' && line[z] != 0)
		z++;
	y = z;
	size = y-x;
	if(size > 250)
		size=250;
	}
else
	{
	size=0;
	x=0;
	}

strncpy(token,&line[x],size);
token[size] = 0;
return(z);
}

initbasic()
{
listenport = 5201;
bancheck = 0;
togglemax = 500;
banhours = 48;
bandeduct = 60;
kwhcost = 0.07;
locks[0] = 60;
locks[1] = 120;
locks[2] = 300;
locks[3] = 600;
locks[4] = 1200;
}

initswitch(struct switchdata *sd)
{
int	x,y,z;
sd->status=0;
sd->port = 0;
sd->switchbase = base;
sd->control = 1;
sd->counter = 0;
sd->watts = 60;
sd->timeused = 0;
sd->timeout = 3*60;
sd->startstatus=1;
sd->stateposition = 0;
for(z=0;z<5;z++)
	sd->times[z] = 0;
sd->maxsecs = 2000*60;
strcpy(sd->name,"");
strcpy(sd->relayhost,"127.0.0.1");
sd->relayport = 5201;
sd->wq = NULL;
}

motorinit(struct motordata *md)
{
int	x,y,z;
for(z=0;z<4;z++)
	md->sequence[z] = 1 << z;
md->numsequence = 4;
md->motorbase = base;
md->currentstep = 25;
md->lastcontact = -1;
md->control = 1;
strcpy(md->name,"");
strcpy(md->relayhost,"127.0.0.1");
md->relayport = 0;
md->wq = NULL;
}

addswitch(struct switchdata sd)
{
struct	sdlink *temp;
struct	sdlink *temp2;

temp = malloc(sizeof *temp);
memcpy(&(temp->sd),&sd,sizeof sd);
temp->next = NULL;
if(switches == NULL)
	switches = temp;
else
	{
	temp2 = switches;
	while(temp2->next != NULL)
		temp2 = temp2->next; 
	temp2->next = temp;
	}
}

addmotor(struct motordata md)
{
struct	mdlink *temp;
struct	mdlink *temp2;

temp = malloc(sizeof *temp);
memcpy(&(temp->md),&md,sizeof md);
temp->next = NULL;
if(motors == NULL)
	motors = temp;
else
	{
	temp2 = motors;
	while(temp2->next != NULL)
		temp2 = temp2->next; 
	temp2->next = temp;
	}
}

int	x10init(struct	X10data  *x10d)
{
int	x,y,z;

x10d->state = 0;
x10d->limit = 0;
x10d->counter = 0;
x10d->timeused = 0;
x10d->watts = 60;
x10d->remotetimeout = 3*60;
x10d->localtimeout = 60*60;
x10d->startstatus = 1;
x10d->type = 1;
x10d->stateposition = 0;
for(z=0;z<5;z++)
	x10d->times[z] = 0;
x10d->maxsecs = 2000 * 60;
x10d->control = 1;
strcpy(x10d->relayhost,"");
x10d->relayport = 0;
x10d->relayhouse = 0;
x10d->wq = NULL;
}

printconfig()
{
struct	sdlink *temp;

printf("This is a display of the data currently in memory.  \n\n");

// First the switches.

temp = switches;
while(temp != NULL)
	{
	printf("[Switch]\n");
	printf("Name: %s\n",temp->sd.name);
	printf("Port: %d\n",temp->sd.port);
	printf("Status: %d\n",temp->sd.status);
	printf("Base: %x\n",temp->sd.switchbase);
	printf("Control: %d\n",temp->sd.control);
	printf("Counter: %d\n",temp->sd.counter);
	printf("Stateposition: %d\n",temp->sd.stateposition);
	printf("Timeout: %d\n",temp->sd.timeout);
	printf("maxsecs: %d\n",temp->sd.maxsecs);
	temp = temp->next;
	}
}

readconfig()
{
FILE	*in;
int	x,y,z;
int	mode;
int	lastmode;
int	housemode;
int	unitmode;
int	times;
char	line[2000];
char	token[256];
char	value[256];
char	s[256];

struct	switchdata	sd;
struct	X10data		x10d;
struct	motordata	md;

switches = NULL;
x10eq = NULL;
initswitch(&sd);
motorinit(&md);
lastmode = 0;
mode=0;
housemode=-1;
unitmode = -1;
times=0;
for(z=0;z<17;z++)
	for(y=0;y<17;y++)
		{
		x10init(&X10mods[z][y]);
		if(z == 0)
			X10mods[z][y].stateposition = y+1;
		}
in = fopen("lampserver.cfg","r");
if(in == NULL)
	{
	printf("The lampserver.cfg file does not exist.  Create one.\n");
	exit(1);
	}
while (!feof(in))
	{
	// read a line from the config file.
	line[0] = 0;
	fgets(line, 1990, in);
	line[1990] = 0;
//	printf("Line: %s\n",line);	
	// parse the line for a comment ; and null the rest of the line.	
	x = strlen(line);
	for(z=0;z<x;z++)
		{
		if(line[z] == ';' || line[z] == '\n')
			{
			line[z] = 0;
			break;
			}
		}
	// get the first token in the line
	x = gettoken(0,line,token);
	// get the second token in the line
	y = gettoken(x,line,value);
	if(strcmp(token,"") == 0)
		continue;
	if(strcmp(token,"[basic]") == 0)
		{
		lastmode = mode;
		initbasic();
		mode = 1;
		}
	if(strcmp(token,"[switch]") == 0)
		{
		lastmode = mode;
		mode = 2;
		times = 0;
		}
	if(strcmp(token,"[x10]") == 0)
		{
		lastmode = mode;
		mode = 3;
		times = 0;
		}
	if(strcmp(token,"[motor]") == 0)
		{
		lastmode = mode;
		mode = 4;
		motorinit(&md);
		}
	if(lastmode)
		{
		if(lastmode == 2)
			{
			addswitch(sd);
			initswitch(&sd);
			}	
		if(lastmode == 4)
			{
			addmotor(md);
			motorinit(&md);
			}
		lastmode = 0;
		}
	if(mode == 1)
		{
		if(strcmp(token,"port") == 0)
			listenport = atoi(value);	
		if(strcmp(token,"togglemax") == 0)
			togglemax = atoi(value);	
		if(strcmp(token,"bancheck") == 0)
			bancheck = atoi(value);	
		if(strcmp(token,"banhours") == 0)
			banhours = atoi(value);	
		if(strcmp(token,"bandeduct") == 0)
			bandeduct = atoi(value);	
		if(strcmp(token,"kwhcost") == 0)
			kwhcost = atof(value);
		}
	if(mode == 2)
		{
		if(strcmp(token,"pin") == 0)
			sd.port = atoi(value);
		if(strcmp(token,"base") == 0)
			sd.switchbase = atoi(value);
		if(strcmp(token,"timeout") == 0)
			sd.timeout = atoi(value);
		if(strcmp(token,"control") == 0)
			sd.control = atoi(value);
		if(strcmp(token,"watts") == 0)
			sd.watts = atoi(value);
		if(strcmp(token,"start") == 0)
			sd.startstatus = atoi(value);
		if(strcmp(token,"time") == 0)
			{
			int	hour;
			int	minute;
			long	minutes;
			hour = atoi(value);
			z=0;
			while(value[z++] != ':');
			minute = atoi(&value[z]);	
			minutes = hour*60+minute;
			sd.times[times++] = minutes;
			}
		if(strcmp(token,"maxsecs") == 0)
			sd.maxsecs = atoi(value);
		if(strcmp(token,"maxmins") == 0)
			sd.maxsecs = atoi(value) * 60;
		if(strcmp(token,"name") == 0)
			{
			strncpy(sd.name,value,19);
			sd.name[19] = 0;
			}
		if(strcmp(token,"relayhost") == 0)
			{
			strncpy(sd.relayhost,value,29);
			sd.relayhost[29] = 0;
			}
		if(strcmp(token,"relayport") == 0)
			sd.relayport = atoi(value);
			
		}
	if(mode == 3)
		{
		int	c1,c2,d1,d2;
		if(strcmp(token,"start") == 0)
			x10d.startstatus = atoi(value);
		if(strcmp(token,"type") == 0)
			x10d.type = atoi(value);
		if(strcmp(token,"time") == 0)
			{
			int	hour;
			int	minute;
			long	minutes;
			hour = atoi(value);
			z=0;
			while(value[z++] != ':');
			minute = atoi(&value[z]);	
			minutes = hour*60+minute;
			x10d.times[times++] = minutes;
			}
		if(strcmp(token,"localtimeout") == 0)
			x10d.localtimeout = atoi(value);
		if(strcmp(token,"remotetimeout") == 0)
			x10d.remotetimeout = atoi(value);
		if(strcmp(token,"maxmins") == 0)
			x10d.maxsecs = atoi(value) * 60;
		if(strcmp(token,"maxsecs") == 0)
			x10d.maxsecs = atoi(value);
		if(strcmp(token,"watts") == 0)
			x10d.watts = atoi(value);
		if(strcmp(token,"relayhost") == 0)
			{
			strncpy(x10d.relayhost,value,29);
			x10d.relayhost[29] = 0;
			}
		if(strcmp(token,"relayport") == 0)
			x10d.relayport = atoi(value);
		if(strcmp(token,"control") == 0)
			x10d.control = atoi(value);
		if(strcmp(token,"house") == 0)
			housemode = atoi(value);
		if(strcmp(token,"unit") == 0)
			unitmode = atoi(value);
		

		if(housemode == -1)
			{
			c1 = 0;
			c2 = 16;
			}
		else
			{
			c1 = housemode;
			c2 = housemode+1;
			}
		if(unitmode == -1)
			{
			d1 = 0;
			d2 = 0;
			}
		else
			{
			d1 = unitmode;
			d2 = unitmode+1;
			}
		for(z=c1;z<c2;z++)
			for(y=d1;y<d2;y++)
				{
				memcpy(&X10mods[z][y],&x10d, sizeof x10d);
				}
		}
	}				
fclose(in);
if(mode == 2)
	{
	addswitch(sd);
	}	
if(mode == 4)
	{
	addmotor(md);
	}
		lastmode = 0;
}



struct	switchdata *searchswitch(char *name)
{
int	x,y,z;
struct	sdlink *temp;

temp = switches;
while(temp != NULL)
	{
	if(strcmp(temp->sd.name,name) == 0)
		return(&temp->sd);
	temp = temp->next;
	}
return(NULL);
}

/* checktimeouts()
This function searches through all linked lists, checks all switches that
have timeouts, and toggles them off if the time has exceeded the max.
*/

int	checktimeouts(struct	rccardata *rccar)
{
int	x,y,z;
char	s[256];
struct	sdlink *temp;
struct	switchdata *temp2;

// first switches
//printf("In checktimeouts.\n");
deletebans();

temp = switches;
//printf("temp: %d\n",temp);
while(temp != NULL)
	{
//	printf("temp: %d   status: %d  time: %d   counter: %d\n",temp,temp->sd.status,time(NULL), temp->sd.counter);
	if(temp->sd.status && time(NULL) > 3*60+temp->sd.counter)
		{
//		printf("Got here.\n");
		temp2 = &(temp->sd);
		lampoff(temp2,getstatevar(temp->sd.switchbase));
		}
	temp = temp->next;
	
	}

if(rccar->wq != NULL)
	{
	printf("There's a host on the queue.  Checking.\n");
	if(rccar->lock)
		{
		printf("Car is locked.  exp: %d  time: %d\n",rccar->lockexp, time(NULL));
		if(rccar->lockexp < time(NULL) || rccar->heartbeat < time(NULL)-60)
			{
			// dequeue if expired or idle for more than 60s.
			rccarwaitdequeue(rccar);
			}
		}
	else
		{
		printf("Dequeueing.\n");
		rccarwaitdequeue(rccar);
		}
		
	}
if(rccar->lock)
	{
	printf("Car is locked.  exp: %d  time: %d\n",rccar->lockexp, time(NULL));
	if(rccar->lockexp < time(NULL) || rccar->heartbeat < time(NULL)-60)
		{
		// dequeue if expired or idle for more than 60s.
		rccarwaitdequeue(rccar);
		}
	}
//printf("Leaving checktimeouts.\n");
}

// This function is called at midnight once a day to reset all timeused
// variables.
int	resettimes()
{
int	x,y,z;
struct	sdlink *temp;
printf("In resettimes.\n");
temp = switches;
while(temp != NULL)
	{
	temp->sd.timeused = 0;
	temp = temp->next;
	}
printf("Leaving resettimes.\n");
}

/* computepower
This function goes through the database and computes the daily power 
consumption and returns an integer representing wattseconds for the day.
*/

int	computepower()
{
int	x,y,z;
int	ws; // wattseconds
float	wh; // watthours
float	f;
struct	sdlink *temp;

ws = 0;

temp = switches;
while(temp != NULL)
	{
	z = (temp->sd.timeused * temp->sd.watts); 
	ws = ws + z;
	temp = temp->next;
	}
for(z=0;z<17;z++)
	for(y=0;y<17;y++)
		{
		x = X10mods[z][y].timeused * X10mods[z][y].watts;
		ws = ws + x;
		}
return(ws);
}

getSTATE(char	*s, int	insanity, int *compstate, int strike)
{
int	x,y,z;
int	secondsleft;
char	tt[100];
long	lt;
struct	tm	*tim;

printf("In state procedure.\n");
s[12+17] = '2';
s[0] = (state & 1) + '0';
for(z=1;z<17;z++)
	{
	s[z] = X10mods[0][z].state + '0';
	}
for(z=0;z<20;z++)
	{
	s[z+17] = compstate[z] + '0';
	}
s[12+17] = '2';
/*
lt = time(NULL);
tim = localtime(&lt);
if( !(tim->tm_hour >= 6 && tim->tm_hour <= 20) || sprinkseconds >= sprinklermax)
	s[12+17] = '2';
else
	s[12+17] = spstatus+'0';

/*
secondsleft = sprinklermax - sprinkseconds;	
if(secondsleft < 0)
*/
	secondsleft=0;
s[37]=(secondsleft/60)/10 + '0';
s[38]=(secondsleft/60)%10 + '0';
s[39]=(secondsleft%60)/10 + '0';
s[40]=(secondsleft%60)%10 + '0';	
sprintf(tt,"%d ",insanity);
strcpy(&s[41],tt);
sprintf(tt,"%d ",strike);
strcat(s,tt);
printf("s len= %d\n",strlen(s));
}


waitqueuelist(char *buf, struct waitqueue *wq)
{
int	x,y,z;
int	secs;
struct	waitqueue *temp;
char	s[80];

temp = wq;
x = 1;
while(temp != NULL)
	{
	secs = time(NULL) - temp->lastupdate;
	sprintf(s,"(%-3d): host: %-16.16s  Last update: %d seconds\n",x,temp->host,secs);
	strcat(buf,s);
	temp = temp->next;
	x++;
	}
}


/* waitqueuestat
This function takes a waitqueue pointer and a host, computes how long that
host will probably have to wait and returns a string formatted:
timeleft, number of users ahead in the queue,

*/
waitqueuestat(char *buf, struct waitqueue *wq, int timeleft, char *host)
{
int	x,y,z;
struct	waitqueue *temp;

z = timeleft;
x = 0;
temp = wq;
while(temp != NULL)
	{
	if(strcmp(temp->host,host) == 0)
		{
		sprintf(buf,"%d	%d",z,x);
		temp->lastupdate = time(NULL);
		return(0);
		}
	x++;	
	if(temp->lastupdate > time(NULL) - 60)
		z = z + 5*60;
	temp = temp->next;
	}
return(1);
}

int	rccarlock(struct rccardata *rccar, char *host)
{
rccarwaitenqueue(host,rccar);
	
return(1);
}

main(argc, argv)
int	argc;
char 	*argv[];
{
int	sock;
int	length;
struct	sockaddr_in server;
struct	sockaddr_in client;
int	msgsock;
char	buf[1024];
char	s[20000],t[1024];
int	x,y,z;
int	rval;
int	i;
time_t	lt;
struct	tm	*tim;
struct	tm 	*lasttim;
FILE	*output;
int	size;
int	lasthour;

char	name[20], command[20], value[20], host[20];

int	lastcontact;	/* stores the last contact tapped */
int	currentstep;	/* What is the current step; for bounding */	

struct	motordata officerotor;	// data for the office cam rotor
struct	rccardata rccar;	// data for the rccar
struct	switchdata officesd;
struct	switchdata *sd;
struct	switchdata sprinksd;

long	counter;	/* Lamp time counter */
long	wcounter;	/* Wheel time counter */
long	spcounter;

int	status; 	/* Lamp status */
int	wstatus;	/* Wheel status */
int	spstatus;

int	lastms;		/* motion sensor tracking */

long	sprinkseconds;	/* number of seconds sprinkler has run today */
long 	sprinklermax;	/* max # of seconds to run the sprinkler each day */
long	starttime;	/* time server was started at, for tracking sprinkler */	
long	lasttime;	/* last time checked */
long	currenttime;	/* time right now */
long	timetmp;

int	insanity;	/* the insanity value */

int	inslamp;	/* insanity increase when a lamp is turned on. */
int	inssub;		/* insanity value to decrease each 10 seconds */

long	insanitytime;

int	shutdownmarker;




int	compstate[20];
long	compchecktime;

fd_set	rfds;
struct	timeval	tv;
int	retval;

unsigned	char	*address_holder;

signal(SIGCHLD,child_handler);
signal(SIGPIPE,pipecleaner);

lasthour = 0;

insanity = 0;
inslamp = 5;
inssub = 10;

lastms = 1;

bans = NULL;

shutdownmarker = 0;

starttime = time(NULL);
insanitytime=starttime;

sprinklermax = 30*60;	/* 15 minutes per day max */
sprinkseconds = sprinklermax;

compchecktime=0;
for(z=0;z<20;z++)
	compstate[z] = 0;

for(z=0;z<17;z++)
	for(y=0;y<17;y++)
		{
		X10mods[z][y].state = 0;
		X10mods[z][y].limit = 0;
		X10mods[z][y].control = 1;
		}

printf("about to read.\n");
readconfig();
printf("Done reading.\n");
printconfig();

status=0;
wstatus=0;
spstatus=0;
state=0;
state2=0;
currentstep=25;
lastcontact=-1;

initswitch(&officesd);
officesd.port = 0;
initswitch(&sprinksd);
sprinksd.port=5;
sprinksd.startstatus = 0;
sprinksd.times[0] = 6*60;
sprinksd.times[1] = 19*60;
sprinksd.maxsecs = 30*60;

officerotor.currentstep=25;
officerotor.lastcontact = -1;
officerotor.sequence[0] = 4;
officerotor.sequence[1] = 8;
officerotor.sequence[2] = 2;
officerotor.sequence[3] = 16;
officerotor.numsequence = 4;
officerotor.motorbase = base;

rccar.commonport=16;
rccar.forwardport=2;
rccar.backwardport=1;
rccar.leftport = 8;
rccar.rightport = 4;
rccar.carbase = base2;
rccar.nudgef = VERYSLOW;
rccar.nudgeb = SLOW;
rccar.pushf = MEDIUM;
rccar.pushb = FAST;
rccar.wstatus = 0;
rccar.wcounter = 0;
rccar.eq = NULL;
rccar.wq = NULL;
rccar.lock = 0;
rccar.lockexp = 0;


sock = sockinit(listenport);
do
	{
	timetmp = time(NULL);
	tim = localtime(&timetmp);	

	msgsock = getconnection(sock);
	if(msgsock > -1)
		{
		strcpy(s,"");
		rval = getcommand(msgsock, buf,name,command,value,host);
		printf("Command: %s %s %s %s\n",name, command,value,host);
		if(rval == -1)
			continue;
		if(strcmp(buf,"ADDBIGINS") == 0)
			{
			printf("Adding insanity.\n");
			insanity = insanity + inslamp*10;
			}
		if(strcmp(buf,"ADDINS") == 0)
			{
			printf("Adding insanity.\n");
			insanity = insanity + inslamp*3;
			}
		if(strcmp(buf,"INSRESET") == 0)
			{
			printf("Restting insanity level.\n");
			insanity=0;
			}
		if(strcmp(buf,"SPRESET") == 0)
			{
			printf("Restting sprinkler timer.\n");
			sprinkseconds=0;
			}
		if(strcmp(buf,"SPCLEAR") == 0)
			{
			printf("Restting sprinkler timer.\n");
			sprinkseconds=60*30;
			}
		if(strcmp(buf,"SHUTDOWN") == 0)
			{
			shutdownmarker = 1;
			}

/*
The next commands are for backwards compatibility until all features and
cgi scripts are updated.
*/
		if(strcmp(buf,"SPON") == 0)
			{
			if( (tim->tm_hour >= 6 && tim->tm_hour <= 20) && sprinkseconds < sprinklermax)
//				lampon(5,&spstatus, &spcounter, &state, &insanity,inslamp);
				lampon(&sprinksd,getstatevar(sprinksd.switchbase), &insanity,inslamp);
			}
		if(strcmp(buf,"SPOFF") == 0)
			{
			/* Turn Sprinkler off */
			lampoff(&sprinksd,getstatevar(sprinksd.switchbase));
			sprinkseconds += time(NULL) - spcounter;
			}
		if(strcmp(buf,"ON") == 0)

			{
			lampon(&officesd, getstatevar(officesd.switchbase), &insanity,inslamp);
			getSTATE(s, insanity, compstate,0);
			}
		if(strncmp(buf,"X10ON",5) == 0)

			{
			x10enqueue(buf,host);
//			x10lampon(buf,&insanity,inslamp);
			getSTATE(s, insanity, compstate,0);
			}
		if(strncmp(buf,"X10OF",5) == 0)

			{
			x10enqueue(buf,host);
		//	x10lampoff(buf);
			getSTATE(s, insanity, compstate,0);
			}

		if(strcmp(buf,"OFF") == 0)
			{
			/* Turn it off, for GOD'S SAKE, turn it OFF!! */
			lampoff(&officesd, getstatevar(officesd.switchbase));
			getSTATE(s, insanity, compstate,0);
			}
		if(strcmp(buf,"LEFT1") == 0)
			{
			pan(1,&officerotor,&state,0);
			getSTATE(s, insanity, compstate,0);
			}
		if(strcmp(buf,"RIGHT1") == 0)
			{
			pan(0,&officerotor,&state,0);
			getSTATE(s, insanity, compstate,0);
			}
		if(strcmp(buf,"CALI") == 0)
			{
			printf("Beginning calibration.\n");
			for(y=0;y<70;y++) 
				{
				pan(0,&officerotor,&state,1);
				usleep(100000);
				}	
			officerotor.currentstep=0;
			for(y=0;y<25;y++) 
				{
				pan(1,&officerotor,&state,1);
				usleep(100000);
				}
			}

		/* Get the last motion sensor tripped */
		if(strcmp(buf,"LASTMS") == 0)
			{
			s[0] = lastms + '0';
			s[1] = 0;
			}

/* This is part of the new routines, just add it later.
*/

		if(strcmp(name,"RCCAR") == 0)
			{
			int strike;
			if(strcmp(command,"NUDGEF") == 0)
				{
				//power(0,1,&rccar, &state2);
				strike = rccarenqueue(0,0,1,host, &rccar);
				
				}
			if(strcmp(command,"PUSHF") == 0)
				{
				//power(1,1,&rccar, &state2);
				strike = rccarenqueue(0,1,1,host, &rccar);
				}
			if(strcmp(command,"NUDGEB") == 0)
				{
				//power(0,0,&rccar, &state2);
				strike = rccarenqueue(0,0,0,host, &rccar);
				}
			if(strcmp(command,"PUSHB") == 0)
				{
				//power(1,0,&rccar, &state2);
				strike = rccarenqueue(0,1,0,host, &rccar);
				}
			if(strcmp(command,"TURNLEFT") == 0)
				{
				//turn(1,&rccar,&state2);
				strike = rccarenqueue(1,0,1,host,&rccar);
				/* Turn Left */
				}
			if(strcmp(command,"TURNRIGHT") == 0)
				{
				//turn(2,&rccar,&state2);
				strike = rccarenqueue(1,0,2,host,&rccar);
				/* Turn Right */
				}
			if(strcmp(command,"CENTER") == 0)
				{
				/* Center wheels */
				//turn(0,&rccar,&state2);
				strike = rccarenqueue(1,0,0,host,&rccar);
				}
			if(strcmp(command,"UPLEFT") == 0)
				strike = rccarenqueue(2,0,5,host, &rccar);
			if(strcmp(command,"UPRIGHT") == 0)
				strike = rccarenqueue(2,0,6,host, &rccar);
			if(strcmp(command,"DOWNLEFT") == 0)
				strike = rccarenqueue(2,0,1,host, &rccar);
			if(strcmp(command,"DOWNRIGHT") == 0)
				strike = rccarenqueue(2,0,2,host, &rccar);
			if(strcmp(command,"LOCK") == 0)
				{
				printf("Locking the RC car for %s\n",host);
				strike = rccarwaitenqueue(host,&rccar);
				}	
			if(strcmp(command,"WAITQUEUE") == 0)
				{
				printf("Getting wait queue for %s\n",host);
				strike = waitqueuestat(s,rccar.wq,rccar.lockexp - time(NULL), host);	
				printf("Got it: .%s.\n",s);
				if(strike)
					{
					if(rccar.lock && strcmp(rccar.lockhost,host) == 0)
						sprintf(s,"%d -1",rccar.lockexp - time(NULL));
					else
						sprintf(s,"-1 -1");
					}
				}
			else if(strcmp(command,"GETWQLIST") == 0)
				{
				if(rccar.lock)
					{
					sprintf(s,"(%-3d): host: %-16.16s  Last update: %d seconds\n",0,rccar.lockhost,time(NULL) - rccar.heartbeat);
					waitqueuelist(s,rccar.wq);
					}
				else
					{
					sprintf(s,"\n");
					}
				}
			else
				{
				printf("Getting state.  Strike = %d\n",strike);
				getSTATE(s, insanity, compstate,strike);
				}
		
			}
		
			
/*
Now for the new routines
*/
		sd = searchswitch(name);
		if(sd != NULL)
			{
			if(strcmp(command,"ON") == 0)
				lampon(sd,getstatevar(sd->switchbase),&insanity,inslamp);
			if(strcmp(command,"OFF") == 0)
				lampoff(sd,getstatevar(sd->switchbase));
			getSTATE(s, insanity, compstate,0);
				
			}

		if(strcmp(buf,"SILENTSTATE") == 0)
			{
			getSTATE(s, insanity, compstate,2);
			}
		if(strcmp(buf,"TIMERESET") == 0)
			{
			resettimes();
			}
		

		if(strcmp(buf,"STATE") == 0)
			{
			getSTATE(s, insanity, compstate,0);
/*
			int	secondsleft;
			char	tt[100];
			printf("In state procedure.\n");
			s[0] = (state & 1) + '0';
			for(z=1;z<17;z++)
				{
				s[z] = X10mods[0][z].state + '0';
				}
			for(z=0;z<20;z++)
				{
				s[z+17] = compstate[z] + '0';
				}
			lt = time(NULL);
			tim = localtime(&lt);
			if( !(tim->tm_hour >= 6 && tim->tm_hour <= 20) || sprinkseconds >= sprinklermax)
				s[12+17] = '2';
			else
				s[12+17] = spstatus+'0';
			
			secondsleft = sprinklermax - sprinkseconds;	
			if(secondsleft < 0)
				secondsleft=0;
			s[37]=(secondsleft/60)/10 + '0';
			s[38]=(secondsleft/60)%10 + '0';
			s[39]=(secondsleft%60)/10 + '0';
			s[40]=(secondsleft%60)%10 + '0';	
			sprintf(tt,"%d",insanity);
			strcpy(&s[41],tt);
			printf("s len= %d\n",strlen(s));
*/
			}
		printf("About to write.\n");
		if(write(msgsock, s, strlen(s)) == -1)
			perror("Writting on stream socket ");
		printf("Done writing.  about to close.\n");
		close(msgsock);
		}
/* This next part is temporary.  For now, it will dequeue one entry from the
rccar queue and execute it, then return to the loop.  This will eventually
be handled by a separate thread.
*/
	if(shutdownmarker)
		{
		close(sock);
		exit(0);
		}
	rccardequeue(&rccar);
	x10dequeue(&insanity, inslamp);




/* This next part will turn off the lamp automatically after it has been
on for 3 minutes straight, to avoid leaving it on all night long. */

	checktimeouts(&rccar);
	

// the old lamp code:
	if(status && time(NULL) > 3*60+counter)
		{
		lampoff(&officesd,getstatevar(officesd.switchbase));
		}



	if(time(NULL) - insanitytime >= 10)
		{
		insanity = insanity - inssub;
		insanitytime = time(NULL);
		if(insanity < 0)
			insanity=0;
		}


	if(spstatus && time(NULL) > 30+spcounter)
		{
		lampoff(&sprinksd,getstatevar(sprinksd.switchbase));
		sprinkseconds += time(NULL) - spcounter;
		}

/* turn off all the X10 lamps after specified time periods */
	for(z=0;z<16;z++)
		{
		for(y=0;y<17;y++)
			{
			if(X10mods[z][y].control && X10mods[z][y].state && time(NULL) > X10mods[z][y].limit*60+X10mods[z][y].counter)
				{
				x10lampoff2(z,y);
/*
				sprintf(s,"/home/pjm/br %c%d off",z+'A',z);
				system(s);	
				X10mods[z][y].state = 0;
*/
				}
			}
		}

/* once per minute, do a pingtest of all the computers and update the 
compstate array */
	
	if(time(NULL) > compchecktime+60)
		{
		int	xxx5;
		
		for(xxx5=0;xxx5 < 10; xxx5++)
			compstate[xxx5] = 1;
		/*
		compstate[0] = pingtest("spaz");
		compstate[1] = pingtest("inferno");
		compstate[2] = pingtest("node5");
		compstate[3] = pingtest("phat");
		compstate[4] = pingtest("node3");
		compstate[5] = pingtest("freedom");
		compstate[6] = pingtest("doorbell");
		compstate[7] = 1; 
		compstate[8] = pingtest("node4");
		compstate[9] = pingtest("oister");
		*/
		compstate[10] = 0;
		compstate[11] = 0;
		compchecktime = time(NULL);
		}	



/* This next part will center the wheels after they have been turned
 for 5 minutes straight, to avoid running down the transmitter battery. */


	if(rccar.wstatus && time(NULL) > 5*60+rccar.wcounter)
		{
		turn(0,&rccar,&state2);
		}
	} while (1);
}
	




int     pingtest(char   *host)
{
int     x,y,z;
char    s[1024];

sprintf(s,"ping -c 1 %s",host);
x = system(s);
if(x)
        {
	return(0);
        }
else
        {
	return(1);
        }
}                            

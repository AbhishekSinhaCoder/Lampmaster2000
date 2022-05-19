/*

This program was written by Paul Mathis (pmathis@nationwide.net).  Please
feel welcome to distribute and use as you wish.  If you make modifications
or improvements, please let me know so I can determine if its useful for
me as well.  

I designed this program based upon my current situation.  I'm running the
client on a 486/66 running linux.  The only purpose of this machine is
to detect the doorbell button being pressed and released, then notifying
my primary linux machine of the event.  Since the joystick does not use
interrupts, the port must be constantly polled to be certain of catching
the button-press.  Therefore, I am dedicating a single machine to the task.
A faster computer would probably be able to handle more.


This program is designed to run under linux.  It waits until it detects
a button pressed on the joystick port, which means the doorbell has been
depressed.  It then waits until the button is released.  At this point,
it sends a message to the doorbell server on another machine, or the local
one via TCP/IP.  

The server then handles logging and other activies related to the doorbell.


The arguments for this program are as follows:
The first argument is the internet address of the doorbell server.
The second argument is the port number of the doorbell server.  The server
program I have included uses port 5200.  This can be changed to anything, 
but both the client and the server must use the same number.  Both these
arguments can be hard coded in the client, but I chose to leave them as 
arguments, since several clients may be used (Multiple doorbells for instance),
but a single server doesn't care where the clients are.

*/







#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>


main(argc, argv)
int	argc;
char 	*argv[];
{
int	sock;
int	rval;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();
char	buf[1024];
char	s[1024];
int	x,y,z;
char	c;

char 	*fname;
int 	fd, status;
struct 	JS_DATA_TYPE js;
int	temp;
int	temp2;
int	state;

fname = "/dev/js0"; 	/* This is the device name for the joystick driver */

/* open device file */
fd = open (fname, O_RDONLY);
if (fd < 0) {
	perror ("js");
	exit (1);
}


temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */
temp2 = 0;	/* Trigger to send a notice of change of door state */
state = -1;	/* initial value to set the state, but not record a change */
while(1)	/* Loop forever. */	
	{
/* first, we want to detect a button press */


	status = read (fd, &js, JS_RETURN); /* read status from joystick */
	if (status != JS_RETURN) {
		perror ("js");
		exit (1);
	}
	/* if we don't know the state of the door, find out. */
	if(state == -1)
		{
		state = js.buttons & 2;
		}

	/* if the button is pressed, but we don't know about it yet */
	if(js.buttons & 1 && temp==0) 
		{
		temp=1; /* button is now currently pressed.  */
		}

	/* if the button is released, but we don't know about it yet */
	if(!(js.buttons & 1) && temp==1)
		{
		temp=2; /* button is no longer pressed */
		}
	if((js.buttons & 2) != state)	/* door state has changed */
		{
		temp2 = 1;	/* send notice of change of state */
		state = js.buttons & 2;
		}
	if(temp != 2 && !temp2)
		{
		/* give other processes a chance */
		usleep (100);
		continue; 
		}


/* At this point the doorbell has been rung, so we send this info to the server */

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		{
		perror("opening stream socket");
		exit(1);
		}
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if(hp == (struct hostent *)0)
		{
		fprintf(stderr,"%s: unknown host", argv[1]);
		exit(2);
		}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	
	if(temp == 2)	/* we detected a doorbell ring. */
		{
		if((connect(sock, &server, sizeof(server)), 0) < 0)
			{
			perror("connecting stream socket");
			exit(1);
			}
		strcpy(s,"DING");
		if((write(sock, s, strlen(s)), 0) < 0)
			perror("writing on stream socket");
	
		if(( rval = read(sock, buf, 1024)) < 0)
			perror("Reading from stream socket");
		}
	if(temp2 == 1)
		{
		if((connect(sock, &server, sizeof(server)), 0) < 0)
			{
			perror("connecting stream socket");
			exit(1);
			}
		if(state > 0)
			strcpy(s,"CLOSE");
		else
			strcpy(s,"OPEN");
		if((write(sock, s, strlen(s)), 0) < 0)
			perror("writing on stream socket");
	
		if(( rval = read(sock, buf, 1024)) < 0)
			perror("Reading from stream socket");
		}
		
/* close the socket after sending the information.  Rings are rare enough to
not require a constant connection. */
	close(sock);

	temp=0; /* back to waiting again */
	temp2=0;
	}
}








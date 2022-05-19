/*
This is the quick rundown of how the program works.

The program connects to an irc server and joins the channel.  It then loops
waiting for server messages.  Most of these messages are written messages
between chatters.  The program specially handles a few of them, but the
rest it passes to the parse function.

The parse function first cleans up the text by converting special characters
to regular english letters as needed and eventually will clean up spelling.
Then it calls the diagram function.  The diagram function without any
knowledge of context will diagram the sentence and place it into a special
structure.  The parse function will then take over and substitute in 
appropriate nouns for all pronouns as needed.  It will then append the
sentence structure to the conversation thread that it fits into. then it
returns to the main loop.

the main loop will constantly poll the conversation thread.  If a new
sentence has been entered into it, it will anaylize the structure to 
determine what to do with it.  If its a command for the bot to carry out,
it will then take action as appropriate.  Otherwise, it will be an
informational sentence and the bot will enter the new information into the
database.  

If the chatter was talking to the bot, the bot will then formulate some
type of response to the text.  If it was a question, it will answer it.
If it was a statement, it will in some way acknowledge it.  Either by 
expressing some type of reaction to it, responding with some relevent
information, or asking a question of its own.  Or even changing the subject
completely. 

For any private conversation that doesn't consist soley of commands, the
bot will attempt to obtain information from the user.  A stock list of 100-200
questions will be available to ask the chatter.  As the chatter responds,
more questions can be developed based on the new data obtained.  The bot
will first attempt to fill holes in its knowledge, then venture into new
territory when it can proceed no longer on its current questioning.

As the bot talks to people, it will look for common patterns.  If it notices
that 2-3 or more people are talking about a certain topic, it will then
form a set of questions based on that topic, then add it to its list of
stock questions.

The bot will remember people and will welcome them back appropriately once its
seen them before.  

There will be two primary databases.  One is world knowledge.  This applies
to ALL the bots, as well as everyone else.  This will mostly be concerned
with how things relate (feet have toes, hands have fingers) that are 
consistant in all cases (with notable exceptions).  These things will be
the same across the board and will never change, so each bot will be able
to access this same wealth of information as it learns it.  In this regard
they collaborate always.

The secondary database is individual knowledge.  This mostly has to do with
the experiences of each individual bot.  Although some bots might encounter
the same people, its not necessary, and not even desired that they share the
same degree of information.  

Use color 16 at the start of any sentence you don't want a bot paying attention
to.  The bots then see this and ignore it.  good for debug messages.
*/





/*
Argv[1] = hostname
[2] = port
[3] = datafile output


*/



#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <math.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/io.h>               
#include <string.h>
#include <mmalloc.h>

#define DATA "half a league, half a league ... "

struct	memory
	{
	char	nick[40];	/* who did it */	
	int	times;		/* how many times? */	
	};

struct	memory2
	{
	char	nick[40];
	char	objects[15][80];
	int	sign[15];
	int	inuse;
	};

struct definfo	// defition info for a word.
	{
	char	word[40];
	char	definition[2000];
	char	type[10];
	};

struct lampinfo
	{
	char	name[40];
	int	type;
	int	unit;
	struct	noun	*nounphrase;
	};

struct	cookietype
	{
	char	name[40];
	char	password[20];
	};

struct	nickinfo
	{
	char	nick[40];
	char	host[120];
	int	gender;	// 0: unknown  1- male, 2-female, 3-bot
	int	opped;
	char	talkingto[40];	//who is this nick currently speaking to?
	int	karma;	// self percieved karma total
	int	reputation;	// other's percieved karma total
	int	online;
	int	greeting;
	long	lasttalktime;	// time of the last communication.
	char	lastobject[200];	// last object spoken of
	char	lastsubject[200];	// last subject spoken of
	};




/*
The object_structure structure defines the basic object, which is a 
nounphrase with several pointers to various lists.  
next_obj is the next different object.
next_np is the next object with the same root noun but different adjectives.
nick is a pointer to the nickinfo structure if this object is a nick.
reverse_vl is a pointer to a verblist.  the verblist is a list of verbs that
  list a list of objects that apply to the main object through that verb.  the
  verblist describes the main object in the passive form. 
forward_vl is the same as reverse_vl except it describes in the active form.
  all verbs are listed in present tense, first person form, with the tense
  variable describing the actual tense.
*/


struct	obj_list
	{
	struct	object_structure	*obj;	
	struct	obj_list	*nextobj;
	};

struct	verb_list
	{
	struct	verb	*vb;
	int	tense;
	struct	verb_list	*next_verb;
	struct	obj_list	*objlist; 
	};

struct	commandlist
	{
	struct	verb	*vb;
	struct	commandlist *next;
	};

struct	object_structure
	{
	struct	noun	*nounphrase;
	struct	object_structure	*next_obj;
	struct	object_structure	*next_np;
	struct	nickinfo *nick;
	struct	verb_list	*reverse_vl;	
	struct	verb_list	*forward_vl;	
	// new fields
	struct	complement	*complist;
	struct	commandlist	*com_list;
	};



struct	rootinfo
	{
	struct	object_structure *things;
	struct	conversation_thread	*cthreads;
	};

/*
This is the info needed for diagramming the sentences.  

- A sentence is the basic structure that is the root of all other links.  
  It maintains a link to the subject and predicate.
- A phrase is a group of words
*/

/*
New structure format.  

Sentence diagramming will take on an automata pattern to digest sentences
and populate a dynamic structure with information.  This structure is then
stored in memory, and eventually to disk.

There will be multiple root structures.  One for the verbs to list the 
subjects of those actions and the objective result.  Another for the
subjects which link to all the stored information about that subject.
Another for individual objects that link back to the subjects they relate
to, and how.
*/

struct	adverb
	{
	char	word[40];
	struct	adverb *adv;
	};

struct	adjective
	{
	char	word[40];
	struct	adjective *adj;	// adjective that describes THIS adjective
	struct	adverb	*adv;	
	
	};

struct	noun
	{
	char	word[40];
	struct	adjective *adj_;	// first adjective
	struct	adjective adj[5];
	struct	prepphrase *pp;
	struct	noun	*nextnoun;
	};

struct	prepphrase
	{
	char	prep[40];
	struct	noun	*nounphrase;
	struct	prepphrase	*pp;
	};

struct	subject
	{
	struct	noun	subnoun;
	struct	noun	*nounphrase;
	};


struct	verb
	{
	char	word[3][40]; // three verbs in a phrase..  has been riding.. 
	struct	adverb	adv;	
	struct	prepphrase	*pp;
	};

struct	object
	{
	struct	noun	objectnoun;
	struct	noun	*nounphrase;
	};

struct	complement
	{
	struct	adjective	*adj;
	};

/*
Predicate consists of a verb, and either an object or a complement.
*/

struct	predicate
	{
	struct	verb	predverb;
	struct	object	predobject;
	struct	object	indirectobject;
	struct	complement	*comp;	
	};
	

struct	sentence
	{
	char	greeting[40];
	char	expletive[40];
	struct subject	sub;
	struct predicate pred;
	};


/*
A conversation thread is created whenever the bot observes a specific 
conversation between two or more people.  This also applies to private
chats between the bot and another chatter.  The thread identifies the 
thread owner (whoever talked first), the audience (either a nick or
the whole channel), and a linked list of sentence structures to recall
what was said.  
*/

struct	sentence_list
	{
	struct	sentence *sent;
	int	marker;	// we have handled this sentence already.
	struct	sentence_list *next;
	int	where;	// 0 for the main channel, 1 for privatechat.
	};

struct	conversation_thread
	{
	char	nick[40];
	char	talkingto[40];
	struct	sentence_list *sentlist;
	struct	conversation_thread	*next;
	};


/*
The basic info structure is all the global variables that change based on
the server or channel that the bot is using.  A single pointer will be 
pointed at the currently selected group of variables at any one time.  The
main loop will then adjust this pointer on each pass to pick up all the
selected structures
*/

struct	basic_info
	{
	};



struct	memory smiles[10];
struct	memory2 details[10];
struct	memory2 possessions[10];
struct	memory monolinux[10];
struct	memory ignores[10];
char	femalelines[40][300];
int	femalelinenum;
char	malelines[40][300];
int	malelinenum;
char	server_string[1024];
int	server_port;
int	respawntoggle;
int	primarytoggle;
char	lastnickhunt[300];
char	botcommander[40];
int	botrank;


struct	nickinfo nickdata[100];
struct	lampinfo	lamps[17];
struct	sent	*lastsent;

char	botnick[40];
char	botchan[40];
int	botgender;
int	silence;
int	debug;
char	preplist[200][20];

char	*quotes[5000];
int	quoteindex[5000];
int	numquotes;
int	pick,ran;
int	privatechat;
char	privchatter[40];

char	*md; //heap pointer
char	*mdtest;
char	mmapfile[80];
int	mmapfilehandle;

struct	rootinfo	*rootdata;


/*
Memorylist will be an array (eventually a ll) of verbs to describe recordable
actions.

*/
struct	memorylist
	{

	};







struct	stackelementtype
	{
	float	number;
	char	operator; // op is 0 if number is used.
	};

struct	stacktype
	{
	struct	stackelementtype stackelement[1000];
	int	size;
	};

int	clearstack(struct stacktype *stack)
{
int	x,y,z;
for(z=0;z<100;z++)
	{
	stack->stackelement[z].number=0;
	stack->stackelement[z].operator = 0;
	stack->size=0;
	}
}

int	checkstack(struct stacktype *stack)
{
return(stack->size);
}

push(float number, char operator, struct stacktype *stack)
{
int	x,y,z;
z = stack->size++;
stack->stackelement[z].number = number;
stack->stackelement[z].operator = operator;
}

pop(float *number, char *operator, struct stacktype *stack)
{
int	x,y,z;
z = --stack->size;
*number = stack->stackelement[z].number;
*operator = stack->stackelement[z].operator;
}


int	isop(char c)
{
if(c == '^')
	return(1);
if(c == '+')
	return(1);
if(c == '-')
	return(1);
if(c == '*')
	return(1);
if(c == '/')
	return(1);
if(c == '(')
	return(1);
if(c == ')')
	return(1);
return(0);
}

int	isdigit(char	c)
{
if(c >= '0' && c <= '9')
	return(1);
else
	if(c == '.')
		return(1);
else
	return(0);
}

float	postfix(char	*eq)
{
float	x,y,z;
int	pos;
int	done;
char	c;
struct	stacktype stack;

clearstack(&stack);
pos=0;
done=0;

while(!done)
	{
	if(isdigit(eq[pos]))
		{
		x = atof(&eq[pos]);
		push(x,0,&stack);
		}
	while(isdigit(eq[pos]))
		pos++;
	while(eq[pos] == ' ')
		pos++;
	if(isop(eq[pos]))
		{
		pop(&x,&c,&stack);
		pop(&y,&c,&stack);
		switch(eq[pos])
			{
			case '+':	z=x+y;
					break;
			case '-':	z=y-x;
					break;
			case '*':	z=x*y;
					break;
			case '/':	z=y/x;
					break;
			}
		push(z,0,&stack);
		pos++;
		}
	if(eq[pos] == 0)
		done=1;
	}
pop(&x,&c,&stack);
return(x);
}


int pri(char	c)
{
switch(c)
	{
	case '^': return 5;
	case '*': return 4;
	case '/': return 3;
	case '+': return 1;
	case '-': return 1;
	}
return(0);
}

int	texttoif(char	*eq,char *neq)
{
int	x,y,z;

}

int	iftopf(char	*eq,char *pf)
{
float	x,y,z;
int	pos;
int	done;
char	c;
char	s[1024];
struct	stacktype stack;

clearstack(&stack);
strcpy(pf,"");
pos=0;
done=0;

while(!done)
	{

	if(isdigit(eq[pos]))
		{
		x = atof(&eq[pos]);
		sprintf(s,"%f ",x);	
		strcat(pf,s);
		}
	while(isdigit(eq[pos]))
		pos++;
	while(eq[pos] == ' ')
		pos++;
	if(isop(eq[pos]))
		{
		if(eq[pos] == '(')
			push(0,'(',&stack);
		else if(!checkstack(&stack) && eq[pos] != ')')
			{
			push(0,eq[pos],&stack);
			}
		else if(eq[pos] == ')')
			{
			c=0;
			while(c != '(')
				{
				pop(&x,&c,&stack);
				if(c != '(')
					{
					sprintf(s,"%c ",c);
					strcat(pf,s);
					}
				}
			}
		else 
			{
			pop(&x,&c,&stack);
			if(c == '(')
				{
				push(x,c,&stack);
				push(0,eq[pos],&stack);
				}
			else if(pri(eq[pos]) > pri(c))
				{
				push(x,c,&stack);
				push(0,eq[pos],&stack);
				}
			else
				{
				sprintf(s,"%c ",c);
				strcat(pf,s);
				pos--;
				}
				
			}	
		}
	if(eq[pos] == 0)
		done=1;
	pos++;
	}
while(checkstack(&stack))
	{
	pop(&x,&c,&stack);
	sprintf(s,"%c ",c);
	strcat(pf,s);
	}
return(x);
}




compute(float *answer, char *eq)
{
int	x,y,z;
char	c;
char	pf[1024];

strcpy(pf,"");
iftopf(eq,pf);
//printf("%s\n",pf);
*answer = postfix(pf);
//printf("Answer: %f\n",postfix(pf));

}


initarguments(int argc, char **argv,char *server, int *port, char *online)
{
int	x,y,z;
char	s[1024];

strcpy(server,"spaz");
strcpy(online,"/home/pjm/spazonline.dat");
*port=6667;
strcpy(botnick,"Jeeves");
strcpy(botchan,"#cam");
strcpy(mmapfile,"Jeeves.memory");
botgender=1;
debug=0;

/*
-s	: server
-n	: nick
-p	: port
-c	: channel
-f	: online stat filename
-d	: debug mode
-m	: memory map filename

*/

z=1;
while(z<argc)
	{
	if(strncmp(argv[z],"-s",2) == 0)
		{
		strcpy(server,&argv[z][2]);
		}
	if(strncmp(argv[z],"-n",2) == 0)
		{
		strcpy(botnick,&argv[z][2]);
		}
	if(strncmp(argv[z],"-p",2) == 0)
		{
		*port = atoi(&argv[z][2]);
		}
	if(strncmp(argv[z],"-c",2) == 0)
		{
		strcpy(botchan,&argv[z][2]);
		}
	if(strncmp(argv[z],"-f",2) == 0)
		{
		strcpy(online,&argv[z][2]);
		}
	if(strncmp(argv[z],"-g",2) == 0)
		{
		strcpy(s,&argv[z][2]);
		if(strcmp(s,"male") == 0)
			botgender=1;
		if(strcmp(s,"female") == 0)
			botgender=2;

		}
	if(strncmp(argv[z],"-d",2) == 0)
		{
		debug=1;
		}
	if(strncmp(argv[z],"-m",2) == 0)
		{
		strcpy(mmapfile,&argv[z][2]);
		}
	z++;
	
	}
}


initquotes()
{
int	x,y,z;
FILE	*handle;
char	s[5000];

handle = fopen("/home/pjm/quotes.txt","r");
z=0;
x=0;
numquotes=0;
printf("loading quotes.\n");
while(!feof(handle))
	{
	fgets(s,4990,handle);
	s[4990] = 0;
	s[strlen(s)-1] = 0;
	if(s[0] == '%')
		{
		quoteindex[numquotes++] = x;
		x=z;
		continue;
		}
	quotes[z] = malloc(strlen(s)+5);
	strcpy(quotes[z],s);
	z++;
	}
quoteindex[numquotes] = z;
printf("loaded %d quotes.\n",numquotes);
sleep(2);

}


make_connection(int	*sockvalue)
{
int	x,y,z;
char	s[1024];
int	sock;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();

printf("Opening socket.\n");
sock = socket(AF_INET, SOCK_STREAM, 0);
if(sock < 0)
	{
	perror("opening stream socket");
	exit(1);
	}
server.sin_family = AF_INET;
//server.sin_len = sizeof(server);
printf("getting host for : %s\n",server_string);
hp = gethostbyname(server_string);
printf("got it.\n");
if(hp == (struct hostent *)0)
	{
	fprintf(stderr,"%s: unknown host", server_string);
	exit(2);
	}
bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
server.sin_port = htons(server_port);

if((connect(sock, &server, sizeof(server)), 0) < 0)
	{
	perror("connecting stream socket");
	exit(1);
	}
printf("\n");
printf("Socket opened.\n");
*sockvalue = sock;
}

botlogin(int sock)
{
char	s[1024];

sprintf(s,"NICK %s\n",botnick);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writting on stream socket");

sprintf(s,"USER %s Paul paul Tammy'sSlave\n",botnick);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writting on stream socket");
}

initvariables(char *lastnick, char *lastques, int *loaded, int *begged, int *joined, int *opped, int *lastopped)
{
int	x,y,z;
char	s[1024];


strcpy(lastnickhunt,"");
for(z=0;z<100;z++)
	{
	strcpy(nickdata[z].nick,"");
	strcpy(nickdata[z].talkingto,"");
	strcpy(nickdata[z].host,"");
	nickdata[z].gender=0;
	nickdata[z].opped=0;
	nickdata[z].karma=0;
	nickdata[z].reputation=0;	
	nickdata[z].online=0;
	nickdata[z].lastobject[0] = 0;
	nickdata[z].lastsubject[0] = 0;
	nickdata[z].greeting=0;
	}

strcpy(lastnick,"");
strcpy(lastques,"");

*loaded = 0;
*begged = 0;
*joined = 0;
*opped = 0;
*lastopped = 0;

for(z=0;z<10;z++)
	{
	strcpy(smiles[z].nick,"");
	smiles[z].times=0;
	}
for(z=0;z<10;z++)
	{
	strcpy(ignores[z].nick,"");
	ignores[z].times=0;
	}
for(z=0;z<10;z++)
	{
	strcpy(monolinux[z].nick,"");
	monolinux[z].times=0;
	}
for(z=0;z<10;z++)
	{
	strcpy(possessions[z].nick,"");
	possessions[z].inuse=0;
	for(y=0;y<15;y++)
		{
		possessions[z].objects[y][0] = 0;
		}
	}
for(z=0;z<10;z++)
	{
	strcpy(details[z].nick,"");
	details[z].inuse=0;
	for(y=0;y<15;y++)
		{
		details[z].objects[y][0] = 0;
		}
	}

respawntoggle=0;

}

save_rootptr()
{
int	x,y,z;
int	handle;

handle = open("rootptr.dat", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
lseek(handle,0,0);
write(handle,&rootdata, sizeof rootdata);
close(handle);
}

load_rootptr()
{
int	x,y,z;
int	handle;

handle = open("rootptr.dat", O_RDONLY); 
lseek(handle,0,0);
read(handle,&rootdata, sizeof rootdata);
close(handle);
}


mdinit()
{
int	newfile;
newfile = 0;

mmapfilehandle = open(mmapfile, O_RDWR);
if(mmapfilehandle == -1)
	{
	newfile = 1;
	mmapfilehandle = open(mmapfile, O_RDWR | O_CREAT | O_TRUNC,S_IRWXU);
	}
md = mmalloc_attach(mmapfilehandle,NULL);
if(newfile)
	{
	rootdata = mmalloc(md,sizeof *rootdata);
	rootdata->things = NULL;
	rootdata->cthreads = NULL;
	save_rootptr();
	}
else
	load_rootptr();

}

main(argc, argv)
int	argc;
char 	*argv[];
{
char	onlinefilename[1024];
int	sock;
int	rval;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();
char	buf[2000];
char	logbuf[20000];
char	s[1024];
char	str[1024];
char	s1[1024],s2[1024];
char	s3[1024],s4[1024];
char	s5[1024],s6[1024];
char	tmpnick[1024];
char	tmphost[1024];
char	lastnick[1024];
char	lastques[1024];
int	x,y,z;
int	retval;
fd_set	rfds;
struct	timeval	tv;
char	c;
FILE	*in;
FILE	*out;
FILE	*slashtxt;
int	logfile;
int	logfilesize;
int	oldlogfilesize;
int	logfile2;
int	logfile2size;
int	oldlogfile2size;
int	pos;
int	oldpos;
int	loaded;
int	joined;
int	opped;
int	begged;
int	oopsopped;
int	lastopped;
long	t,oldt;
int	nickfound;
int	nicknum;
int	slashfork;
int	freshfork;
int	forked;
int	totalnick;
int	wordnum;
int	loopdone;
int	nickstat[20];
char	nicks[20][20];
int	inuse;
char	lastslash[2000];
char	lastfresh[2000];
long	slashtime,oldslashtime;
long	freshtime,oldfreshtime;



strcpy(botnick,"Jeeves");
strcpy(botchan,"#cam");
primarytoggle=1;
silence =0;
debug=0;
forked=0;
initquotes();
initarguments(argc, argv,server_string, &server_port, onlinefilename);
printf("server: %s   port: %d   filename: %s   botnick: %s   botchan: %s\n",server_string, server_port, onlinefilename, botnick, botchan);

strcpy(botcommander,botnick);
botrank=1;
mdinit();

printf("Allocated at %d\n",md);
sleep(2);
printf("Testing.\n");
initvariables(lastnick, lastques, &loaded, &begged, &joined, &opped, &lastopped);

slashfork=freshfork=-1;
oopsopped=0;
lastslash[0] = 0;
time(&oldslashtime);
time(&oldfreshtime);
for(z=0;z<17;z++)
	{
	lamps[z].unit=-1;
	lamps[z].type=-1;
	lamps[z].nounphrase = NULL;
	strcpy(lamps[z].name,"");	
	}
printf("about to open.\n");
in=fopen("/home/pjm/malelines.txt","r");
z=0;
while(!feof(in))
	{
	fgets(s,300,in);
	s[299] = 0;
	s[strlen(s)-1] = 0;
	strcpy(malelines[z],s);
	printf("male line: %s\n",s);
	z++;		
	}
malelinenum=z;
fclose(in);

in=fopen("/home/pjm/femalelines.txt","r");
z=0;
while(!feof(in))
	{
	fgets(s,300,in);
	s[299] = 0;
	s[strlen(s)-1] = 0;
	strcpy(femalelines[z],s);
	printf("female line: %s\n",s);
	z++;		
	}
femalelinenum=z;
fclose(in);


in=fopen("/home/pjm/preplist.txt","r");
z=0;
while(!feof(in))
	{
	preplist[z][0] = 0;
	fgets(preplist[z],19,in);
	preplist[z][19] = 0;
	x = strlen(preplist[z]);
	if(preplist[z][x-1] == '\n')
		preplist[z][x-1] = 0;
	z++;
	}
fclose(in);
in=fopen("/home/pjm/lampconfig.cfg","r");		
while(!feof(in))
	{
	fgets(s,1020,in);
	printf("s: %s\n",s);
//	sscanf(s,"%d;%s;%d;%d",z,s1,x,y);
	y=0;
	x=0;
	
	z=atoi(s);
	while(s[x] != ';')
		x++;
	x++;	
	y=x;
	while(s[x] != ';')
		x++;
	strncpy(s1,&s[y],x-y);
	s1[x-y]=0;
	strcpy(lamps[z].name,s1);
	x++;
	lamps[z].type=atoi(&s[x]);
	while(s[x] != ';')
		x++;
	x++;
	lamps[z].unit=atoi(&s[x]);
	printf("scanned.\n");
	}
printf("done loading.\n");
fclose(in);
lampdiagram();
printf("Loaded lamps.\n");
for(z=0;z<17;z++)
	{
	if(lamps[z].type == -1)
		continue;
//	getnp(s,lamps[z].nounphrase);
	printf("#: %d   Name: %s  Type: %d  Unit: %d\n",z,lamps[z].name, lamps[z].type,lamps[z].unit);
//	printf("Nounphrase: %s\n",s);
	
	}



oldslashtime = oldslashtime - 120;
oldfreshtime = oldfreshtime - 120;
strcpy(nicks[0],"Restil");
strcpy(nicks[1],"Tx_Sweetheart");
strcpy(nicks[2],"Restil2");
strcpy(nicks[3],"PovRayMan");
strcpy(nicks[4],"Crispee");
strcpy(nicks[5],"Texas_Chik");
strcpy(nicks[6],"SlashChick");
strcpy(nicks[7],"Corky");
strcpy(nicks[8],"bunny");
strcpy(nicks[9],"Leslie");
strcpy(nicks[10],"Dj12midnit");
strcpy(nicks[11],"Emily");
strcpy(nicks[12],"Aimee");
strcpy(nicks[13],"Nikita");
strcpy(nicks[14],"wickedbecca");
strcpy(nicks[15],"Tammy");
strcpy(nicks[16],"oops");
strcpy(nicks[17],"dredge");

totalnick=18;

make_connection(&sock);

botlogin(sock);



sleep(5);
pos=0;
oldpos=0;
time(&t);
oldt=t;
nickfound=0;
nicknum=0;

logfile = open("/tmp/mesglog.txt",O_RDONLY);
oldlogfilesize=logfilesize = lseek(logfile,0,2);
close(logfile);
logfile2 = open("/var/lib/httpd/cgi-bin/dblogs.txt",O_RDONLY);
oldlogfile2size=logfile2size = lseek(logfile2,0,2);
close(logfile2);


printf("At the while loop.\n");
printf("Joined: %d\n",joined);
while(1)
	{
	privatechat = 0;
	if(respawntoggle)
		{
		mmalloc_detach(md);
		close(mmapfilehandle);

		close(sock);
		make_connection(&sock);

		mdinit();

		botlogin(sock);
		if(respawntoggle==2)
			{
			initvariables(lastnick, lastques, &loaded, &begged, &joined, &opped, &lastopped);
			primarytoggle=0;
			botrank=2;
			debug=1;
			time(&t);
			oldt = t;
			}
		else
			{
			loaded=0;
			begged=0;
			joined=0;
			opped=0;
			lastopped=0;
			time(&t);
			oldt = t;
			respawntoggle=0;
			}
		}
	time(&t);
	retval=0;
        FD_ZERO(&rfds);
        FD_SET(sock,&rfds);
/* Wait 1 second for data, then proceed with other stuff */

        tv.tv_sec=0;
        tv.tv_usec=1;
        retval = select(11, &rfds, NULL, NULL, &tv);
        if(retval)              /* we got data */        
		{
	
		if(( rval = read(sock, &buf[pos], 1024)) < 0)
			perror("Reading from stream socket");

		buf[rval+pos] = 0;	
//		printf("Data: .%s.\n",buf);
		}
	while(!getline(buf,str))
		{
//		printf("String: .%s.\n",str);
		sscanf(str,"%s %s %s %s %s %s\n",s1,s2,s3,s4,s5,s6);
		if(strcmp(s1,"PING")==0)
			{
			printf("Ping.  Pong.\n");
			sprintf(s,"PONG %s\n",&s2[1]);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
			}
		if(strcmp(s2,"KICK")==0)
			{
			joined=0;
			}
		if(strcmp(s2,"PART") == 0 || strcmp(s2,"QUIT") == 0)
			{
			int	xxx;
			for(xxx=1;s1[xxx] != '!' && s1[xxx] != 0;xxx++)
				tmpnick[xxx-1] = s1[xxx];
			tmpnick[xxx-1]=0;
			xxx = addnick(tmpnick);
			nickdata[xxx].online=0;
			nickdata[xxx].opped=0;
			if(CIgrepitw(tmpnick,"Jeeves") && CIgrepitw(botnick,"Trixie"))
				{
				// Commander is gone.  Assume command.
				debugsayit("I assume the bot commander position.",sock);
				primarytoggle=1;
				}
			}
		if(strcmp(s2,"JOIN") == 0)
			{
			int	xxx;
			int	yyy;
			for(xxx=1;s1[xxx] != '!' && s1[xxx] != 0;xxx++)
				tmpnick[xxx-1] = s1[xxx];
			tmpnick[xxx-1]=0;
			if(s1[xxx] == '!')
				{
				for(yyy=xxx+1;s1[yyy] != ' ' && s1[yyy] != 0;yyy++)
					{
					tmphost[yyy-xxx-1] = s1[yyy];
					}
				tmphost[yyy-xxx-1]=0;
				}
			else
				strcpy(tmphost,"");
			xxx = addnick(tmpnick);

			printf("nick: %s  host: %s\n",tmpnick,tmphost);
			if(strcmp(tmpnick,botnick) != 0)
				{
				if(botgender == 1)
					{
					if(femalename(tmpnick))
						femalegreeting(sock,tmpnick);	
					else
						normalgreeting(sock,tmpnick);	
					}
				else if(botgender == 2)
					{
					if(malename(tmpnick))
						malegreeting(sock,tmpnick);	
					else
						normalgreeting(sock,tmpnick);	
					}
				else
					normalgreeting(sock,tmpnick);	
				nickdata[xxx].greeting=1;
				nickdata[xxx].opped=0;
				strcpy(nickdata[xxx].host,tmphost);
				if(CIgrepitw(tmpnick,"Jeeves") && primarytoggle)
					{
					debugsayit("My commander has arrived.  I reliquish bot command.",sock);
					sayit("Jeeves, assume command",sock);
					primarytoggle=0;
					}
				}
			
			}
		if(strcmp(s2,"NICK") == 0)
			{
			int	xxx;
			int	yyy;
			for(xxx=1;s1[xxx] != '!' && s1[xxx] != 0;xxx++)
				tmpnick[xxx-1] = s1[xxx];
			tmpnick[xxx-1]=0;
			if(s1[xxx] == '!')
				{
				for(yyy=xxx+1;s1[yyy] != ' ' && s1[yyy] != 0;yyy++)
					{
					tmphost[yyy-xxx-1] = s1[yyy];
					}
				tmphost[yyy-xxx-1]=0;
				}
			else
				strcpy(tmphost,"");

			for(xxx=1;xxx<strlen(str) && str[xxx] != ':';xxx++);
			strcpy(s3,&str[xxx+1]);
			s3[strlen(s3)-1] = 0;
			// tmpnick changed his nick to s3.
			fixmemory(tmpnick,s3,smiles);
			fixmemory(tmpnick,s3,monolinux);
			fixmemory2(tmpnick,s3,details);
			fixmemory2(tmpnick,s3,possessions);
			if(similar("Restil",s3))
				{	
				sayit("That name is too similar to Restil.  Change it.",sock);
				}
			if(strncmp(tmpnick,"Anonymous_User",14) == 0)
				{
				int	anonnum;
				struct	cookietype cook;
				int	handle;
				anonnum = atoi(&tmpnick[14]);
				printf("This was an anonymous user.  His number was %d.  His new name is .%s.\n",anonnum,s3);
				sprintf(s,"Thanks %s.  I will update your account with your new name.\n",s3);
				sayit(s,sock);
				handle = open("/var/lib/httpd/cgi-bin/cookies/newcookie.dat",O_RDWR);
				strncpy(cook.name,s3,39);
				cook.name[39] = 0;
				cook.password[0] = 0;
				lseek(handle,anonnum* sizeof cook,0);
				write(handle,&cook,sizeof cook);
				close(handle);
				}
			if(CIgrepits(s3,"monolinux"))
				{
				addmemory(s3,"monolinux",monolinux);
				sayit("Bad idea bud!",sock);
				}
			if(botgender == 1)
				{
				if(femalename(s3))
					{
					femalegreeting(sock,s3);	
					}
				}
			else if(botgender == 2) 
				{
				if(malename(s3))
					{
					malegreeting(sock,s3);	
					}
				}
			}
// Uncomment this line to get the return codes
//		printf("S2: %s\n",s2);
		if(strcmp(s2,"PRIVMSG") == 0)
			{
			int	xxx;
			int	yyy;
			char	holder[1024];
			char	temps4[1024];
			strcpy(holder,tmpnick);
			for(xxx=1;s1[xxx] != '!' && s1[xxx] != 0;xxx++)
				tmpnick[xxx-1] = s1[xxx];
			tmpnick[xxx-1]=0;
			if(s1[xxx] == '!')
				{
				for(yyy=xxx+1;s1[yyy] != ' ' && s1[yyy] != 0;yyy++)
					{
					tmphost[yyy-xxx-1] = s1[yyy];
					}
				tmphost[yyy-xxx-1]=0;
				}
			else
				strcpy(tmphost,"");
			if(strcmp(tmpnick,holder) != 0)
				strcpy(lastnick,holder);
			for(xxx=1;xxx<strlen(str) && str[xxx] != ':';xxx++);
			strcpy(s4,&str[xxx+1]);
			s4[strlen(s4)-1] = 0;
			xxx=0;	
			yyy=0;
			while(s4[xxx] != 0)
				{
				if(s4[xxx] == 3)
					{
					xxx++;
					while(s4[xxx] >= '0' && s4[xxx] <= '9')
						xxx++;
					}
				else if(s4[xxx] == 1)
					xxx++;
				temps4[yyy] = s4[xxx];		
				xxx++; yyy++;
				}
			temps4[yyy]=0;
			strcpy(s4,temps4);
			if(CIgrepit(s4,"ACTION"))
				{
				for(xxx=0;xxx<strlen(s4);xxx++)
					printf("%d ",s4[xxx]);
				printf("\n");
				}
			if(strcmp(s3,botchan) == 0 || isnick(s3))
				{
				if(strcmp(s3,botchan) == 0)
					{
					privatechat=0;
					}
				else
					{
					privatechat=1;
					strcpy(privchatter,tmpnick);
					}
				printf("A nick: .%s.\n",tmpnick);
				if(CIgrepits(tmpnick,"monolinux"))
					{
					printf("Found monolinux.\n");
					addmemory(tmpnick,"monolinux",monolinux);
					printf("cm: %d\n",checkmemory(tmpnick,monolinux));
					if(checkmemory(tmpnick,monolinux) == 1)
						{
						sprintf(s,"%s, change your nick or stop talking.\n",tmpnick);
						sayit(s,sock);
						}
					if(checkmemory(tmpnick,monolinux) == 2)
						{
						sprintf(s,"I'm warning you for the last time %s, that nick has to go.\n",tmpnick);
						sayit(s,sock);
						}
					if(checkmemory(tmpnick,monolinux) == 3)
						{
						sprintf(s,"Forget it.  You're not worth the effort of reforming.  Begone!\n",tmpnick);
						sayit(s,sock);
						sprintf(s,"KICK %s %s\n",botchan,tmpnick);
						if((write(sock, s, strlen(s)), 0) < 0)
							perror("writting on stream socket");
						clearmemory(tmpnick,monolinux);
						
						}
					
					}
				if(CIgrepit(s4,"What do you want from me?"))
					{
					sayit("I don't want anything from you!",sock);
					}
				if(CIgrepit(s4,"I love you guys"))
					{
					sayit("We love you too!!!",sock);
					}
				if(CIgrepit(s4,"a/s/l"))
					{
					sayit("What are we?  14???  GROW UP already!",sock);
					}
				if(CIgrepit(s4,"define") && primarytoggle)
					{
					strcpy(s5,"");
					strcpy(s6,"");
					sscanf(s4,"define %s %s",s5,s6);
					printf("s4: %s, s5: %s, s6: %s\n",s4,s5,s6);
					
					if(atoi(s6) == 0)
						wordnum=1;
					else
						wordnum = atoi(s6);	
					sprintf(s,"Defining %s, number %d",s5,wordnum);
					sayit(s,sock);
					sprintf(s,"/usr/bin/dict -h inferno -d wn %s > /home/pjm/dictoutput.txt",s5);
					system(s);
					printf("extracting.\n");
					extract_def(wordnum,1,sock);
					continue;
					}
/*
				if(CIgrepit(s4,"turn off"))
					{
					deactivate(s4,sock,lamps);
					}
				if(CIgrepit(s4,"turn on"))
					{
					activate(s4,sock,lamps);
					}
*/				
				if(CIgrepit(s4,"hehe") || CIgrepit(s4,"lol") || CIgrepit(s4,"rotf") || CIgrepit(s4,":)"))
					{
					addmemory(tmpnick,"smiles",smiles);
					if(checkmemory(tmpnick,smiles) >= 4)
						{
						clearmemory(tmpnick,smiles);
						
						sprintf(s,"%s you sure seem happy today.\n",tmpnick);
						debugsayit(s,sock);
						if((write(sock, s, strlen(s)), 0) < 0)
							perror("writting on stream socket");
						
						}	
					}
				if(CIgrepit(s4,"ACTION") && (CIgrepit(s4,"smiles") || CIgrepit(s4,"laughs")))
					{
					addmemory(tmpnick,"smiles",smiles);
					if(checkmemory(tmpnick,smiles) >= 4)
						{
						clearmemory(tmpnick,smiles);
						sprintf(s,"%s you sure seem happy today.\n",tmpnick);
						debugsayit(s,sock);
						
						}	
					}
				if(CIgrepit(s4,"Thanks dude! I owe you one."))
					{
					sprintf(s,"Oh %s, quit sucking up!\n",tmpnick);
					sayit(s,sock);
					}
				if(CIgrepits(s4,"monolinux") || CIgrepitw(s4,"fap") || CIgrepit(s4,"fap "))
					{
					addmemory(tmpnick,"monolinux",monolinux);
					if(checkmemory(tmpnick,monolinux) == 1)
						{
						sprintf(s,"%s, it would be in your best interest if you NEVER SAID THAT AGAIN!!!!!\n",tmpnick);
						sayit(s,sock);
						}
					if(checkmemory(tmpnick,monolinux) == 2)
						{
						sprintf(s,"I'm warning you for the last time %s, don't speak the forbidden words!\n",tmpnick);
						sayit(s,sock);
						}
					if(checkmemory(tmpnick,monolinux) >= 3)
						{
						sprintf(s,"Forget it.  You're not worth the effort of reforming.  Begone!\n",tmpnick);
						sayit(s,sock);
						sprintf(s,"KICK %s %s\n",botchan,tmpnick);
						if((write(sock, s, strlen(s)), 0) < 0)
							perror("writting on stream socket");
						clearmemory(tmpnick,monolinux);
						
						}
					
					}
				if((CIgrepit(s4,"go away") || CIgrepit(s4,"leave")) && CIgrepit(s4,botnick))
					{
					sayit("But I don't wanna leave!",sock);
					}
				if(CIgrepit(s4,"lights") && CIgrepit(s4,"bother you"))
					{
					sprintf(s,"PRIVMSG %s :I think I can answer that.  Restil is not bothered by the lights.  Would he intentionally hook up a bunch of internet controlled lamps if he thought it was going to bother him?  Do YOU wake up every day thinking \"Hmm.. I wonder what I can do today to annoy myself!!!!\"  Didn't think so.\n",botchan);
					if((write(sock, s, strlen(s)), 0) < 0)
						perror("writting on stream socket");
					}
				parse_text(tmpnick,lastnick,s4,sock,lamps,lastques,details,possessions);	
				}

			if(strcmp(s3,botnick) == 0)
				{
				printf("string: .%s.\n",s4);
				if(strcmp(s4,"PASSWORD") == 0)
					{
					printf("The magic word has been spoken by %s\n",tmpnick);
					sprintf(s,"MODE %s +o %s\n",botchan,tmpnick);
					if((write(sock, s, strlen(s)), 0) < 0)
						perror("writting on stream socket");
					
					}
				}
			}
		if(strcmp(s2,"MODE")==0)
			{
			int	xxx;
			sscanf(s1,":%s!",tmpnick);
			for(xxx=0;xxx<strlen(tmpnick);xxx++)
				if(tmpnick[xxx] == '!')
					{
					tmpnick[xxx] = 0;
					break;
					}
			printf("mode change by %s\n",tmpnick);
			if(strcmp(s4,"-o") == 0)
				{
				int j;
				printf("Deopping %s  %s.\n",s4,s5);
				j = addnick(s5);
				printf("Value = %d  nick: %s  opped: %d\n",j,nickdata[j].nick,nickdata[j].opped);
				nickdata[j].opped = 0;	
				printf("Value = %d  nick: %s  opped: %d\n",j,nickdata[j].nick,nickdata[j].opped);
				}
			if(strcmp(s4,"+o") == 0)
				{
				int j;
				j = addnick(s5);
				nickdata[j].opped = 1;	
				}
			if(strcmp(s3,botchan) == 0 && strcmp(s4,"+o") == 0 && strcmp(s5,botnick) == 0)
				{
				opped=1;
				sprintf(s,"PRIVMSG %s :Thank you %s.\n",botchan,tmpnick);
				if((write(sock, s, strlen(s)), 0) < 0)
					perror("writting on stream socket");
				begged = 0;
				}
			if(strcmp(s3,botchan) == 0 && strcmp(s4,"-o") == 0 && strcmp(s5,botnick) == 0)
				{
				opped=0;
				sprintf(s,"PRIVMSG %s :%s, That's SO wrong.\n",botchan,tmpnick);
				if((write(sock, s, strlen(s)), 0) < 0)
					perror("writting on stream socket");
				}
			}
	//	printf("S1: %s   S2: %s\n",s1,s2);
		x = atoi(s2);
		if(x == 376)
			{
			printf("Loaded motd.\n");
			loaded=1;
			}	
		if(x == 422)
			{
			printf("Missing motd.\n");
			loaded=1;
			}	
		if(x == 311)
			{
			int	xxx;
			sscanf(str,"%s %s %s %s %s %s\n",s1,s2,s3,s4,s5,s6);
//			printf("%s was found.\n",s4);
			xxx=addnick(s4);	
			strcpy(nickdata[xxx].host,s6);
			nickfound=1;
			}
		if(x == 353)
			{
			int i,j,k;
			int opp;
			int	nickstart;
			i=0;
			j=0;
			nickstart=0;
			printf("found channel nicks.  Working on the list now.\n");
			printf("string: .%s.\n",str);
			while(!nickstart)
				{
				if(str[i] == ' ')
					j++;
				i++;
				if(j == 5)
					nickstart=1;
				}
			printf("j = %d, i=%d\n",j,i);
			nickstart=0;
			k=i;
			opp=0;
			while(!nickstart)
				{
				if(str[i] == ' ' || str[i] == 0)
					{
					printf("Found space at %d\n",i);
					if(str[i] == 0)
						nickstart=1;
					strncpy(s4,&str[k],i-k);
					s4[i-k] = 0;
					i++;
					k = i;
					if(s4[0] == ':')
						{
						strcpy(s5,&s4[1]);
						}
					else if(s4[0] == '@')
						{
						opp = 1;
						strcpy(s5,&s4[1]);
						}
					else
						strcpy(s5,s4);
					printf("Found nick: %s\n",s5);
					j = addnick(s5);
					sprintf(s,"WHOIS %s\n",s5);
					if((write(sock, s, strlen(s)), 0) < 0)
						perror("writting on stream socket");
					if(opp)
						nickdata[j].opped = 1;	
					
					}	
				i++;
				opp=0;
				}
			
			}	
		if(x == 401)
			{
			sscanf(str,"%s %s %s %s\n",s1,s2,s3,s4);
//			printf("%s was not found.\n",s4);
			nickfound=-1;
			}
			
		}
	pos=getline(buf,str);
	if(pos == -1)
		pos = 0;
//	printf("pos %d\n",pos);
/*
	if(pos == oldpos && pos > 0)
		{
		printf("buf: %s\n",buf);
		buf[pos] = '\n';
		buf[pos] = 0;	
		}
*/
	oldpos=pos;
	if(loaded && !joined)
		{
		sprintf(s,"JOIN %s\n",botchan);
		if((write(sock, s, strlen(s)), 0) < 0)
			perror("writting on stream socket");
		joined=1;
		printf("Joined the channel.  loaded = %d, joined = %d, opped = %d, begged = %d\n",loaded,joined, opped, begged);
		}
	if(loaded && joined && !opped && !begged)
		{
		//wait 30 seconds to beg.
//		printf("Waiting to beg.  t-oldt: %d\n",t-oldt);
		if(t-oldt > 30)
			{
			if(CIgrepitw(botchan,"#cam"))
				{
				if(botgender == 1)
					sayit("Trixie, op me.",sock);
				else
					sayit("Jeeves, op me.",sock);
				}
			begged=1;
			}
		}
	if(loaded==1 && pos==0 && t-oldt > 60 && primarytoggle)
		{
		if(nicknum < totalnick && nickfound==0)
			{
//			printf("Starting search for Nick #%d.\n",nicknum);
			nickfound=2;
			sprintf(s,"WHOIS %s\n",nicks[nicknum]);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
			}
		if(nickfound == 1)
			{
//			printf("Found nick #%d\n",nicknum);
			nickstat[nicknum] = 1;
			nickfound = 0;
			nicknum++;
			}
		if(nickfound == -1)
			{
//			printf("Found nick #%d\n",nicknum);
			nickstat[nicknum] = 0;
			nickfound = 0;
			nicknum++;
			}
		if(nicknum >= totalnick )
			{
//			printf("Writing Nick File.\n");
			oldt = t;
			nicknum = 0;
			while(1)
				{
				sprintf(s,"rm %s",onlinefilename);
				system(s);
				out = fopen(onlinefilename,"w");
				if(out == NULL)
					{
					perror("bad file problem.");
					exit(1);
					}
				else
					break;
				}
			for(z=0;z<totalnick;z++)
				{
				if(nickstat[z])
					fprintf(out,"%s is online.\n",nicks[z]);
				else
					fprintf(out,"%s is offline.\n",nicks[z]);
				}
			fclose(out);
//			printf("Done searching for nicks.\n");
			}
		}
	loopdone = 0;
	while(!loopdone) {
	logfile = open("/tmp/mesglog.txt",O_RDONLY);
	logfilesize = lseek(logfile,0,2);
	if(logfilesize > oldlogfilesize)
		{
		lseek(logfile,oldlogfilesize,0);
		x = read(logfile,logbuf,19990);
		logbuf[x] = 0;
		while(!getline(logbuf,str))
			{
/*
			sprintf(s,"PRIVMSG Restil :%s : %d %d %d\n",str,oldlogfilesize,logfilesize,x);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
			sprintf(s,"PRIVMSG Texas_Chik :%s\n",str);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
*/
			if(primarytoggle)
				{
				sprintf(s,"%s",str);
				sayit(s,sock);
				}
			}
		oldlogfilesize=logfilesize;
		}
	else
		loopdone=1;
	close(logfile);
	}
	loopdone=0;
	while(!loopdone) {
	logfile2 = open("/var/lib/httpd/cgi-bin/dblogs.txt",O_RDONLY);
	logfile2size = lseek(logfile2,0,2);
	if(logfile2size > oldlogfile2size)
		{
		lseek(logfile2,oldlogfile2size,0);
		x = read(logfile2,logbuf,19990);
		logbuf[x] = 0;
		while(!getline(logbuf,str))
			{
			sprintf(s,"PRIVMSG Restil :%s : %d %d %d\n",str,oldlogfilesize,logfilesize,x);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
			sprintf(s,"PRIVMSG %s :%s\n",botchan,str);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
			}
		oldlogfile2size=logfile2size;
		}
	else
		loopdone=1;
	close(logfile2);
	}
	/*
	if(!forked)	
		{
		if(fork() == 0)
			{
			while(1)
				{
				sleep(10);
				time(&slashtime);
				checklink("http://slashdot.org/slashdot.rdf","A new slashdot article has been posted",&slashtime,&oldslashtime,sock,lastslash);
				time(&freshtime);
				checklink("http://freshmeat.net/backend/fm.rdf","A new software project has been posted on Freshmeat.net",&freshtime,&oldfreshtime,sock,lastfresh);
				}
			}
		else
			forked = 1;
		}
	*/
	}

	
close(sock);
}

int	getline(buf,str)
char	buf[];
char	str[];
{
int	x,y,z;
z=0;
x = strlen(buf);
if(x == 0)
	return(-1);
for(z=0;buf[z] != '\n' && z < x;z++);
if(z >= x) 
	{
	return x;
	}
strncpy(str,buf,z);
str[z]=0;
for(y=z+1;y<=x;y++)
	buf[y-(z+1)] = buf[y];
return 0;
}

int	searchfor(char *buf, char *str, int p)
{
int	x,y,z;
int	sizebuf;
int	sizestr;

sizebuf = strlen(buf);
sizestr = strlen(str);
for(z=p;z<sizebuf-sizestr;z++)
	{
	if(strncmp(&buf[z],str,sizestr) == 0)
		{
		return(z);
		}
	}
return(-1);
}


checklink(char	*xmllink,char *noticestr,long *currenttime, long *oldtime,int sock,char *lastslash)
{
char	s[1024];
int	slashpos;
int	pos;
char	slashtmp[1024];
int	searchsize;
FILE	*slashtxt;

if(*currenttime > *oldtime+(5*60))
	{
	printf("Got here.\n");
	strcpy(slashtmp,lastslash);
	system("rm slashout.txt");
	sprintf(s,"wget --output-document=slashout.txt %s",xmllink);
	system(s);
	system("cat slashout.txt | grep \"<title>\" > slashdata.txt");
	slashtxt = fopen("slashdata.txt","r");
	fgets(s,1020,slashtxt);
	fgets(s,1020,slashtxt);
	fgets(s,1020,slashtxt);
	s[1020] = 0;
	fclose(slashtxt);
	pos = searchfor(s,"<title>",0);	
	if(pos != -1)
		{	
		for(slashpos=7+pos;s[slashpos] != '<' && s[slashpos] != 0;slashpos++);
		strncpy(slashtmp,&s[7+pos],slashpos-pos-7);
		slashtmp[slashpos-pos-7] = 0;
		slashtmp[900] = 0;
		}
	if(strcmp(lastslash,slashtmp)	!= 0) 
	/* Found a new slashdot article */
		{
		strcpy(lastslash,slashtmp);
		sprintf(s,"PRIVMSG %s :%s:  %s\n",botchan,noticestr,slashtmp);
		if((write(sock, s, strlen(s)), 0) < 0)
			perror("writting on stream socket");
		system("cat slashout.txt | grep \"<link>\" > slashdata.txt");
		slashtxt = fopen("slashdata.txt","r");
		fgets(s,1020,slashtxt);
		fgets(s,1020,slashtxt);
		fgets(s,1020,slashtxt);
		s[1020] = 0;
		fclose(slashtxt);
		pos = searchfor(s,"<link>",0);	
		if(pos != -1)
			{

			for(slashpos=6+pos;s[slashpos] != '<' && s[slashpos] != 0;slashpos++);
			strncpy(slashtmp,&s[6+pos],slashpos-6-pos);
			slashtmp[slashpos-6-pos] = 0;
			slashtmp[900] = 0;
			sprintf(s,"PRIVMSG %s :Link:  %s\n",botchan,slashtmp);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
			}

		}
	*oldtime = *currenttime;
	system("rm slashout.txt");
		
	}	
}

upcasestr(char *buf)
{
int	x,y,z;

for(z=0;z<strlen(buf);z++)
	{
	if(buf[z] >= 'a' && buf[z] <= 'z')
		buf[z] = buf[z] - 'a' + 'A';
	}
}



int CIgrepitw(char *buf, char *str)
{
int	x,y,z;
int	bufsize;
int	strsize;
char	s[10000];
char	t[1024];

strcpy(s,buf);
strcpy(t,str);
bufsize=strlen(s);
strsize=strlen(t);
upcasestr(s);
upcasestr(t);
if(strcmp(s,t) == 0)
	return(1);
return(0);
}


int CIgrepit(char *buf, char *str)
{
int	x,y,z;
int	bufsize;
int	strsize;
char	s[10000];
char	t[1024];

//printf("searching for %s in %s.\n",str,buf);
strcpy(s,buf);
strcpy(t,str);
bufsize=strlen(s);
strsize=strlen(t);
upcasestr(s);
upcasestr(t);
for(z=0;z<bufsize;z++)
	{
	if(strncmp(&s[z],t,strsize) == 0)
		{
//		printf("found %s it.\n",t);
		return(1);
		}
	}
return(0);
}

int CIgrepits(char *buf, char *str)
{
int	x,y,z;
int	bufsize;
int	strsize;
char	q[1024];
char	s[10000];
char	t[1024];

//printf("searching for %s in %s.\n",str,buf);
strcpy(s,buf);
strcpy(t,str);
bufsize=strlen(s);
strsize=strlen(t);
upcasestr(s);
upcasestr(t);
for(z=0;z<bufsize;z++)
	{
	strncpy(q,&s[z],strsize);
	q[strsize]=0;
	if(similar(q,t))
		{
//		printf("found %s it.\n",t);
		return(1);
		}
	}
return(0);
}

/*
Eventually adapt this and other memory structures to use a relational 
database.  This will allow not only for unlimited entries, but also searching
across databases


*/
addmemory(char *nick, char *table, struct memory *memories)
{
int	x,y,z;
int	done;
int	id;
char	temps[1024];
char	s[50],t[50];

// First check to find a blank memory or if it already exists.

done=0;
for(z=0;z<10;z++)
	{
	if(CIgrepitw(memories[z].nick,nick))
		{
		memories[z].times++;
		done=1;
		break;
		}

	if(memories[z].times == 0)
		{
		strcpy(memories[z].nick,nick);
		memories[z].times = 1;
		done=1;
		break;
		}
	}
if(!done) // no available slot.  Make one
	{
	for(z=1;z<10;z++)
		{
		strcpy(memories[z-1].nick,memories[z].nick);
		memories[z-1].times = memories[z].times;
		}	
	strcpy(memories[9].nick,nick);
	memories[9].times = 1;
	}
}

int	checkmemory(char *nick, struct memory *memories)
{
int	x,y,z;
int	done;
char	s[50],t[50];

// First check to find a blank memory or if it already exists.
done=0;
for(z=0;z<10;z++)
	{
	if(CIgrepitw(memories[z].nick,nick))
		{
		return(memories[z].times);
		}
	}
return(-1);
}

int	fixmemory(char *oldnick, char *newnick,struct memory *memories)
{
int	x,y,z;
int	done;
char	s[50],t[50];

done=0;
for(z=0;z<10;z++)
	{
	if(CIgrepitw(memories[z].nick,oldnick))
		{
		strcpy(memories[z].nick,newnick);
		}
	}
return(-1);
}

int	fixmemory2(char *oldnick, char *newnick,struct memory2 *memories)
{
int	x,y,z;
int	done;
char	s[50],t[50];

done=0;
for(z=0;z<10;z++)
	{
	if(CIgrepitw(memories[z].nick,oldnick))
		{
		strcpy(memories[z].nick,newnick);
		}
	}
return(-1);
}

int	clearmemory(char *nick, struct memory *memories)
{
int	x,y,z;
int	done;
char	s[50],t[50];

// First check to find a blank memory or if it already exists.
done=0;
for(z=0;z<10;z++)
	{
	if(strcmp(memories[z].nick,nick) == 0)
		{
		memories[z].times = 0;
		}
	}
return(-1);
}

debugsayit(char *str, int sock)
{
char	s[1024];
char	target[40];

if(debug == 1)
	return(-1);
if(silence == 1 && !privatechat)
	return(-1);
if(privatechat)
	strcpy(target,privchatter);
else
	strcpy(target,botchan);	
printf("Bot is Talking to %s\n",target);
sprintf(s,"PRIVMSG %s :%s\n",target,str);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writting on stream socket");
}

sayit(char *str,int sock)
{
char	s[1024];
char	target[40];

printf("Sayit function.\n");
if(silence == 1 && !privatechat)
	return(-1);
if(privatechat)
	strcpy(target,privchatter);
else
	strcpy(target,botchan);	
printf("Saying %s to %s.  Sock=%d\n",str,target,sock);
sprintf(s,"PRIVMSG %s :%s\n",target,str);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writting on stream socket");
}

tellit(char *str, char *nick, int sock)
{
char	s[1024];
sprintf(s,"PRIVMSG %s :%s\n",nick, str);
printf("About to write.\n");
if(send(sock,s,strlen(s),0) < 0)
	perror("writting on stream socket");
/*
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writting on stream socket");
*/
printf("Done Written.\n");
}

int	spitfile(char	*filename, char *nick,int sock)
{
int	x,y,z;
FILE	*spit;
char	txtline[1024];

y=0;
spit = fopen(filename,"r");
while (!feof(spit))
	{
	txtline[0] = 0;
	fgets(txtline,1020,spit);
	txtline[1020]=0;
	tellit(txtline,nick,sock);
	}
fclose(spit);
}

int	extract_def(int	wordnum, int mode, int	sock)	
{
int	x,y,z;
char	s[1024];
char	s1[1024],s2[1024],s3[1024];
int	done;
int	complete;
int	semi;
int	retval;
char	word[100];
char	wordtype[40];
int	numdefs;
FILE	*spit;
char	txtline[1024];
struct	definfo defs[40];	// 40 defintions should be adaquate.


y=0;
done=0;
numdefs=0;
retval=0;
defs[0].type[0] = 0;
spit = fopen("/home/pjm/dictoutput.txt","r");
//printf("opened.\n");
fgets(txtline,1020,spit);
if(strncmp(txtline,"No def",6) == 0)
	{
	// no definitions found
	if(mode==1)
		sayit("No dice dude.",sock);
	return(0);
	}
//printf("Found something.\n");
fgets(txtline,1020,spit);
fgets(txtline,1020,spit);
fgets(txtline,1020,spit);
fgets(txtline,1020,spit);
z=0;
//printf("Entering while loop.\n");
while(txtline[z+2] != 0)
	{
	word[z] = txtline[z+2];
	z++;
	}
word[z-1] = 0;
//printf("Word: .%s.\n",word);

/*
Now we have the word.  The remaining output is in the following format:
First, it tells what kind of word it is: n, v, adj, etc.  This is in position 7
The word type is followed by a : if only one definition is included.  otherwise
  its followed by 1: 
a definition ends with a ;  Everything after that is an example. 
defitinions may wrap to the next line, indented over to line up with the 
  previous's lines defition.
*/
complete=0;
semi=0;
numdefs=-1;
while (!feof(spit) && !done)
	{
	txtline[0] = 0;
	fgets(txtline,1020,spit);
	txtline[1020]=0;
	if(txtline[7] == ' ' && strlen(txtline) >= 7) // not a first line.
		{
		if(semi) 
			continue; // def is done, looking for next line.
		// this is the second line of a definition.  
		for(y=x;txtline[y] != ';' && txtline[y] != 0;y++);
		z = strlen(defs[numdefs].definition);
		defs[numdefs].definition[z] = ' ';
		strncpy(&defs[numdefs].definition[z+1],&txtline[x],y-x);
		defs[numdefs].definition[y-x+z] = 0;
		if(txtline[y] == ';')
			{
			semi=1;
			}
		}
	else  // this IS a first line.  Not sure what type yet.
		{
		semi = 0; 
		numdefs++;
		defs[numdefs].definition[0] = 0;
		
		sscanf(txtline,"      %s %s %s;",s1,s2,s3);
		if(atoi(s1) == 0) //this line has a word type.
			{
			x = 7+strlen(s1) + 1 + strlen(s2) + 1;
			strcpy(wordtype,s1);
//			printf("s1: .%s.  s2: .%s.  s3: .%s.  marker = %d: .%c.\n",s1,s2,s3,x,txtline[x]);
			}
		else // This line has a #: starting instead of a word type
			{
			x = 7+strlen(s1)+1;
			}
		strcpy(defs[numdefs].word,word);
		strcpy(defs[numdefs].type,wordtype);
		for(y=x;txtline[y] != ';' && txtline[y] != 0 && txtline[y] != '\n';y++);
		strncpy(defs[numdefs].definition,&txtline[x],y-x);
		defs[numdefs].definition[y-x] = 0;
		if(txtline[y] == ';')
			semi = 1;
		// y is now the location of the ; or the end of the line.
		}
//	printf("complete: %d   string currently: %s\n",complete,defs[numdefs].definition);
	if(feof(spit))
		{
		done=1;
		}
	}
if(wordnum > numdefs)
	wordnum = 1;
sprintf(s,"You asked me to define %s.  Its a ",defs[wordnum-1].word);
if(strcmp(defs[wordnum-1].type,"n") == 0)
	{
	retval=4;
	strcat(s,"noun");
	}
if(strcmp(defs[wordnum-1].type,"v") == 0)
	{
	retval=2;
	strcat(s,"verb");
	}
if(strcmp(defs[wordnum-1].type,"adv") == 0)
	{
	retval=5;
	strcat(s,"adverb");
	}
if(strcmp(defs[wordnum-1].type,"adj") == 0)
	{
	retval=3;
	strcat(s,"adjective");
	}
strcat(s,".  The definition: ");
strcat(s,defs[wordnum-1].definition);	 
if(mode==1)
	{
	sayit(s,sock);
	sprintf(s,"There are %d definitions total for %s",numdefs,word);
	sayit(s,sock);
	}
fclose(spit);
return(retval);
}

spittest(int	sock)
{
int	x,y,z;
char	buf[20000];

buf[0]=0;
for(z=0;z<20;z++)
	sprintf(buf,"PRIVMSG %s :Channel Spamming tests for fun and profit.\n",botchan);
if(write(sock, buf, strlen(buf)) < 0)
	perror("writting on stream socket");

}

flirtwith(char	*nick, int	sock)
{
int	x,y,z;
char	s[1024];

printf("I'm flirting with %s\n",nick);
ran = rand();
if(botgender==1)
	{
	pick = (int)(((float)malelinenum*ran)/(RAND_MAX+1.0));
	printf("pick: %d\n",pick);
	sprintf(s,"Hey %s: %s\n",nick,malelines[pick]);
	}
else if(botgender==2)
	{
	pick = (int)(((float)femalelinenum*ran)/(RAND_MAX+1.0));
	printf("pick: %d\n",pick);
	sprintf(s,"Hey %s: %s\n",nick,femalelines[pick]);
	}
sayit(s,sock);
}





// The fun of the program.  AI!!!
parse_text(char	*nick, char *lastnick, char *buf,int sock, struct lampinfo *lamps,char *lastques, struct memory2 *memories,struct memory2 *possessions)
{
int	x,y,z;
int	numwords;
int	pos;
int	negate;
int	asked;
int	sentence_type;	// 0 - statement. 1 - question.  2- order
int	ques;
int	quality;
int	possession;
int	order;
int	called;
int	nickindex;
char	s[1024];
char	subject[1024];
char	inter[1024];
char	verb[1024];
char	object[1024];
char	words[30][40];
char	revealnick[40];
char	ts1[50],ts2[50],ts3[50];
struct	sentence sent;
struct	noun *subnp;
struct	noun *objnp;
struct  verb *vb;

if(checkmemory(nick,ignores) > 0 && !privatechat)
	return(0);
// first, split the sentence up into separate words.
for(z=0;z<10;z++)
	words[z][0] = 0;
negate=0;
quality=0;
possession=0;
order=0;
called=0;
sentence_type=0;
nickindex = addnick(nick);
time(&nickdata[nickindex].lasttalktime);
x = suckwords(buf,words);
if(x==10)
	{
	debugsayit("Lamerspeak detected!!!!",sock);
	sprintf(s,"He meant to say: %s",buf);
	debugsayit(s,sock);
	}
initsentence(&sent);
if(!CIgrepitw(words[0],"compute"))
	ques = diagram(buf,lastques,&sent);
printf("Completed diagram\n");

//sscanf(buf,"%s %s %s %s %s %s %s %s %s %s",words[0],words[1],words[2],words[3],words[4],words[5],words[6],words[7],words[8],words[9]);
pos=0;
for(z=0;z<30;z++)
	if(words[z][0] == 0)
		break;
numwords=z;
// Special cases first
printf("Starting special cases.\n");
if(CIgrepitw(words[0],"compute"))
	{
	float	fl;
	strcpy(s,&buf[8]);
	printf("%s %s",s,buf);
//	compute(&fl,s);
	sprintf(s,"Sorry.. I'm braindead at the moment.  Try again later.");
	sprintf(s,"%.2f",fl);
	sayit(s,sock);
	return(1);
	}
if(CIgrepitw(words[0],"pingtest"))
	{
	if(CIgrepitw(words[1],"everything"))
		{
		pingtest("router",sock);
		pingtest("node3",sock);
		pingtest("node4",sock);
		pingtest("node5",sock);
		pingtest("spaz",sock);
		pingtest("doorbell",sock);
		pingtest("oister",sock);
		pingtest("inferno",sock);
		pingtest("freedom",sock);
		pingtest("phat",sock);
		}
	else
		pingtest(words[1],sock);
	freesentence(&sent,NULL);
	return(1);
	}
if(CIgrepit(buf,"tell me about"))
	{
	printf("Special case for telling about stuff.\n");
	pos=0;
	while(!CIgrepit(words[pos],"about"))
		{	
		pos++;
		}
	pos++;
	// current word is the nick.  There are a few special words to check for.
	asked=0;
	if(CIgrepitw(words[pos],"myself"))
		{
		asked=1;
		strcpy(revealnick,nick);
		}
	else if(CIgrepitw(words[pos],"him"))
		{
		strcpy(revealnick,lastnick);
		}
	else if(CIgrepitw(words[pos],"you"))
		{
		strcpy(revealnick,lastnick);
		}
	else if(CIgrepitw(words[pos],"her"))
		{
		strcpy(revealnick,lastnick);
		}
	else if(CIgrepitw(words[pos],"it"))
		{
		strcpy(revealnick,lastnick);
		}
	else if((CIgrepitw(words[pos],"the") && CIgrepitw(words[pos+1],"lamps")) || (CIgrepitw(words[pos],"lamp") && CIgrepitw(words[pos+1],"status")))
		{
		saylampstatus(sock,lamps);
		freesentence(&sent,NULL);
		return(1);
		}
	else
		strcpy(revealnick,words[pos]);
	printf("Position: %d.  word: %s.\n",pos,revealnick);
	reveal_info(revealnick,	sock,asked,memories,possessions);
	freesentence(&sent,NULL);
	return(1);
	}
// obtain the subject
sentence_type = ques;
if(ques == -1)
	{
	freesentence(&sent,NULL);
	return(-1);
	}
printf("about to start sentence type: %d\n",sentence_type);
if(sentence_type == 1)
	{
subnp = sent.sub.nounphrase;
objnp = sent.pred.predobject.nounphrase;
vb = cpverb(&sent.pred.predverb,NULL);

getsubj(ts1,sent);
printf("Subject: %s\n",ts1);
if(sent.expletive[0] != 0)
	{
	strcpy(nickdata[nickindex].talkingto,sent.expletive);
	}
if(CIgrepitw(ts1,"I"))
	{
	// the subject is the speaking user.
	strcpy(subject,nick);
	}
else if(CIgrepitw(ts1,"you"))
	{
	printf("Filling the YOU value.\n");
	// the subject is the user who last spoke.
	if(nickdata[nickindex].talkingto[0] == 0)
		strcpy(subject,lastnick);
	else
		strcpy(subject,nickdata[nickindex].talkingto);
	printf("I chose %s\n",subject);
	}
else if(CIgrepitw(ts1,"she"))
	{
	// the subject is the last female speaker, or last female subject
	strcpy(subject,lastnick);
	}
else if(CIgrepitw(ts1,"he"))
	{
	// the subject is the last male speaker, or last male subject
	strcpy(subject,lastnick);
	}
else if(CIgrepitw(ts1,"it"))
	{
	// the subject is the last bot speaker, or last thing subject
	strcpy(subject,lastnick);
	}
else
	{
	// running out of all other options, just take the word.
	strcpy(subject,ts1);
	}	
	updateobject(nick,lastnick,sent);
	getobject(ts3,sent);
	printf("Object: %s\n",ts3);
	if(CIgrepitw(ts3,"mine"))
		{
		sprintf(object,"%s's",nick);
		}
	else if(CIgrepitw(ts3,"yours"))
		{
		sprintf(object,"%s's",lastnick);
		}
	else if(CIgrepitw(ts3,"hers"))
		{
		sprintf(object,"%s's",lastnick);
		}
	else if(CIgrepitw(ts3,"his"))
		{
		sprintf(object,"%s's",lastnick);
		}
	else
		strcpy(object,ts3);
printf("Asking if %s is %s\n",subject,object);
	getverb(ts2,sent);
	if(isindc(ts2))
		{

		x = addmemory2(-1,subject,object,memories);
		}
	if(isposv(ts2))
		x = addmemory2(-1,subject,object,possessions);
	printf("Verb: %s\n",ts2);
	strcpy(sent.sub.nounphrase->word,subject);

	/* create a function to take a string and build the structure,
	  instead of just using words */

	buildsent(lastques,sent);
	printf("Built sentence: .%s.\n",lastques);
	//sprintf(lastques,"%s %s %s",subject,ts2,object);
	if(CIgrepitw(subject,object))
		{
		sayit("Yes",sock);
		}
	else if(x == 2)
		{
		sayit("Yes",sock);
		}
	else if(x == 1)
		sayit("No",sock);
	else
		{
	//	sayit("I don't know.",sock);
		}
	
		
		
freesentence(&sent,NULL);
return(1);
}
if(sentence_type == 0) {
if(sent.expletive[0] != 0)
	{
	printf("expletive!!!!\n");
	called=1;
	strcpy(nickdata[nickindex].talkingto,sent.expletive);
	}
subnp = sent.sub.nounphrase;
objnp = sent.pred.predobject.nounphrase;
vb = cpverb(&sent.pred.predverb,NULL);

getsubj(ts1,sent);
printf("Got subject.\n");

if(CIgrepitw(ts1,"I") || CIgrepitw(ts1,"ACTION"))
	{
	// the subject is the speaking user.
	strcpy(subject,nick);
	strcpy(subnp->word,nick);
	}
else if(CIgrepitw(ts1,"you"))
	{
	// the subject is the user who last spoke.
	if(nickdata[nickindex].talkingto[0] == 0)
		strcpy(subject,lastnick);
	else
		strcpy(subject,nickdata[nickindex].talkingto);
	strcpy(subnp->word,subject);
	printf("Talking to %s\n",subject);
	}
else if(CIgrepitw(ts1,"she"))
	{
	// the subject is the last female speaker, or last female subject
	strcpy(subject,lastnick);
	strcpy(subnp->word,subject);
	}
else if(CIgrepitw(ts1,"he"))
	{
	// the subject is the last male speaker, or last male subject
	strcpy(subject,lastnick);
	strcpy(subnp->word,subject);
	}
else if(CIgrepitw(ts1,"it"))
	{
	// the subject is the last bot speaker, or last thing subject
	strcpy(subject,lastnick);
	strcpy(subnp->word,subject);
	}
else if(CIgrepitw(ts1,"who"))
	{
	// A question asking which nicks relate to the object
	strcpy(subject,"who");
	strcpy(subnp->word,subject);
	}
else
	{
	// running out of all other options, just take the word.
	strcpy(subject,ts1);
	}	
pos++;
	}
getverb(ts2,sent);
printf("Verb: %s\n",ts2);
//if(CIgrepitw(ts2,"am") || CIgrepitw(ts2,"is") || CIgrepitw(ts2,"are"))
if(isindc(ts2))
	{
	// This means we assign information to the subject
	quality = 1;
	printf("a quality verb.\n");
	}
else if(isposv(ts2))
	{
	possession=1;
	printf("A possession verb.\n");
	}
else if(CIgrepitw(ts2,"turn") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order=1;
	printf("An order.\n");
	}
else if(CIgrepitw(ts2,"hush") && (CIgrepitw(subject,botnick) || privatechat))
	{
	printf("This command is for %s\n",subject);
	order=1;
	printf("An order.\n");
	}
else if(CIgrepitw(ts2,"speak") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order=1;
	printf("An order.\n");
	}
else if(CIgrepitw(ts2,"flirt") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order = 1;
	}
else if(CIgrepitw(ts2,"ignore") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order = 1;
	}
else if(CIgrepitw(ts2,"respawn") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order = 1;
	}
else if(CIgrepitw(ts2,"assume") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order = 1;
	}
else if(CIgrepitw(ts2,"spawn") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order = 1;
	}
else if(CIgrepitw(ts2,"list") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order = 1;
	}
else if(CIgrepitw(ts2,"op") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order = 1;
	}
else if(CIgrepitw(ts2,"die") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order=1;
	}
else if(CIgrepitw(ts2,"kill") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order=1;
	}
else if(CIgrepitw(ts2,"quote") && (CIgrepitw(subject,botnick) || privatechat))
	{
	order = 1;
	}

else
	{
	// give up, we don't know anything else right now.
	printf("Subject: .%s.  ts2: .%s. Talking to %s\n",subject, ts2,nickdata[nickindex].talkingto);
	if(subject[0] == 0 && ts2[0] == 0 && (called || sent.greeting[0] != 0) && CIgrepitw(nickdata[nickindex].talkingto,botnick))
		{
		printf("Got here.\n");
		if(sent.greeting[0] != 0)
			{
			// we have a greeting.
			switch(nickdata[nickindex].greeting)
				{
				case 0:
					sprintf(s,"Hello %s",nick);
					sayit(s,sock);
					nickdata[nickindex].greeting=1;
					break;
				case 1:
					sprintf(s,"How are you %s?",nick);
					sayit(s,sock);
					nickdata[nickindex].greeting++;
					break;
				}
			}
		else
			sayit("Yes?",sock);
		}
	freesentence(&sent,NULL);
	return(0);
	}
pos++;
getadverb(ts3,sent);
printf("Adverb: %s\n",ts3);
if(CIgrepitw(ts3,"not"))
	{
	negate = 1;
	pos++;
	}
updateobject(nick,lastnick,sent);
getobject(ts3,sent);
printf("ObjecT: %s\n",ts3);
if(sentence_type == 0)
	{
	if(CIgrepitw(ts3,"mine"))
		{
		sprintf(object,"%s's",nick);
		strcpy(objnp->word,object);
		}
	else if(CIgrepitw(ts3,"yours"))
		{
		if(nickdata[nickindex].talkingto[0] == 0)
			sprintf(object,"%s's",lastnick);
		else
			strcpy(object,nickdata[nickindex].talkingto);
		strcpy(objnp->word,object);
		}
	else
		strcpy(object,ts3);
/*
	if(numwords > pos+1)
		{
		printf("too many words.\n");
		return(0);
		}
*/
	}
else if(sentence_type == 2)
	{
	printf("Checking: .%s.\n",words[pos]);
	if(CIgrepitw(words[pos],"I"))
		{
		sprintf(s,"You are %s.",nick);
		sayit(s,sock);
		freesentence(&sent,NULL);
		return(1);
		}	
	}
if(CIgrepitw(subject,"who"))
	{
	int	foundit;

	buildsent(lastques,sent);
	strcpy(s,"");
	if(CIgrepit(object,"opped"))
		{
		z=0;
		while(nickdata[z].nick[0] != 0)
			{
			printf("Nickdata: Nick: %s, opped: %d\n",nickdata[z].nick, nickdata[z].opped);
			if(nickdata[z].opped==1)
				{
				strcat(s,nickdata[z].nick);
				strcat(s," ");
				}
			z++;
			}
		sayit(s,sock);
		freesentence(&sent,NULL);
		return(0);
		}
	for(z=0;z<10;z++)
		{
		foundit=0;
		for(y=0;y<15;y++)
			{
			if(negate)
				{
				if(CIgrepitw(memories[z].objects[y],object) && memories[z].sign[y] == 0)
					{
					strcat(s,memories[z].nick);
					strcat(s," ");
					
					}
				}
			else
				{
				if(CIgrepitw(memories[z].objects[y],object) && memories[z].sign[y] == 1)
					{
					strcat(s,memories[z].nick);
					strcat(s," ");
					}
				}
			}
		}
	if(s[0] != 0)
		{
		sayit(s,sock);
		}
	else
		{
		sprintf(s,"I don't know anyone that is %s.",object);
		sayit(s,sock);
		}
	freesentence(&sent,NULL);
	return(1);
			
	}
if(order)
	{
	char	ts4[1024];
	char	ts5[1024];
	char	answers[10][200];
	printf("I got an order.\n");
	getprep(ts3,sent.pred.predverb.pp);
	if(CIgrepitw(ts2,"turn"))
		{
		if(ts3[0] == 0)
			{
			freesentence(&sent,NULL);
			return(0);
			}
		printf("prep: %s\n",ts3);
		if(CIgrepitw(ts3,"on") || CIgrepitw(ts3,"off"))
			{
			int	lampnum;
			int	mode;
			// turn a lamp on or off
			getprepobject(ts4,sent.pred.predverb.pp);
			printf("object: %s\n",ts4);
			strcpy(ts5,"");
			if(CIgrepitw(ts3,"on"))
				mode = 1;
			else
				mode=0;
			if(CIgrepitw(ts4,"debug"))
				{
				debug=!mode;
				sprintf(s,"debug mode is %s.",ts3);
				sayit(s,sock);
				freesentence(&sent,NULL);
				return(0);
				}
			picklamp(mode,sent.pred.predverb.pp->nounphrase,answers);
			lampnum=0;
			if(answers[0][0] == 0)
				strcpy(answers[0],ts4);
			while(answers[lampnum][0] != 0)
				{
				printf("picklamp result was: %s\n",answers[0]);
				strcpy(ts5,answers[lampnum]);
				if(CIgrepitw(ts3,"on"))
					activate(ts5,sock,lamps);
				if(CIgrepitw(ts3,"off"))
					deactivate(ts5,sock,lamps);
				lampnum++;
				}
			freesentence(&sent,NULL);
			return(0);
			}	
		}	
	if(CIgrepitw(ts2,"hush"))
		{
		if(nickdata[nickindex].opped)
			{
			sayit("Ok.  I'll shut up for now.",sock);
			silence = 1;
			}
		else
			{
			sayit("Uhhhh... NO!  MUHAHAHAHAHHAA.",sock);
			}
		}
	if(CIgrepitw(ts2,"quote"))
		{
		int xxx;
		printf("Quoting.\n");
		ran = rand();
		pick = (int)(((float)numquotes*ran)/(RAND_MAX+1.0));
		printf("Pick: %d\n");
		xxx=quoteindex[pick];
		while(xxx < quoteindex[pick+1])
			sayit(quotes[xxx++],sock);	
		}
	if(CIgrepitw(ts2,"speak"))
		{
		silence = 0;
		sayit("Ok, I'm talking again.",sock);
		}
	if(CIgrepitw(ts2,"flirt"))
		{
		if(sent.pred.predverb.pp != NULL)
			{
			getprepobject(ts4,sent.pred.predverb.pp);
			flirtwith(ts4,sock);	
			}
		else
			{
			// pick a random person and flirt with them.
			}
		}
	if(CIgrepitw(ts2,"die"))
		{
		if(nickdata[nickindex].opped)
			{
			printf("nick: %s, nickindex = %d, opped= %d\n",nick,nickindex,nickdata[nickindex].opped);
			sayit("So long everyone.  Its been fun.\n",sock);
			close(sock);
			exit(0);
			}
		else
			{
			sprintf(s,"Sorry %s, you don't have permission to kill me.",nick);
			sayit(s,sock);
			}

		}
	if(CIgrepitw(ts2,"respawn"))
		{
		if(nickdata[nickindex].opped)
			{
			sayit("Ok. brb.",sock);
			respawntoggle=1;
			}
		}
	if(CIgrepitw(ts2,"op"))
		{
		int	xxx;
		getobject(ts3,sent);
		translateobject(ts3,s,lastnick, nick,nickindex);
		strcpy(ts3,s);
		xxx = addnick(ts3);
		if(nickdata[xxx].host[0] == 0)
			{
			sayit("Try again.  I'm slow.\n",sock);
			return(0);
			}
		printf("Host is: .%s.\n",nickdata[xxx].host);
		// first, if the asker is an op, we do so automatically.
		if(nickdata[nickindex].opped)
			{
			sprintf(s,"MODE %s +o %s\n",botchan,ts3);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
			}
		// otherwise...  if the object is authorized, op it.
		else if(CIgrepit(nickdata[xxx].host,"alignment.net"))
			{
			sprintf(s,"MODE %s +o %s\n",botchan,ts3);
			if((write(sock, s, strlen(s)), 0) < 0)
				perror("writting on stream socket");
			}
		else
			{
			sayit("I'm afraid I can't do that.",sock);
			}
		return(0);	
		}
	if(CIgrepitw(ts2,"assume"))
		{
		getobject(ts3,sent);
		if(CIgrepitw(ts3,"command"))
			{
			if(nickdata[nickindex].opped || CIgrepitw(nick,"Trixie"))
				{
				sayit("I assume bot command.",sock);
				primarytoggle=1;
				}
			
			}
		}
	if(CIgrepitw(ts2,"kill"))
		{
		
		}
	if(CIgrepitw(ts2,"list"))
		{
		printf("Got list command.\n");
		getobject(ts3,sent);
		listobjects(sock);
		}
	if(CIgrepitw(ts2,"spawn"))
		{
		getobject(ts3,sent);
		if(CIgrepitw(ts3,"Jeeves") || CIgrepitw(ts3,"Trixie"))
			{
			int	xxx;
			if(CIgrepitw(ts3,botnick))
				{
				sayit("Uhh, that's me.  I'm already here. I don't need a clone, thanks.\n",sock);
				return(0);
				}
			xxx = fork();
			if(xxx == 0)
				{
				if(CIgrepitw(ts3,"Jeeves"))
					{
					strcpy(botnick,"Jeeves");
					botgender=1;
					respawntoggle=2;
					strcpy(mmapfile,"Jeeves.memory");
					
					return(0);
					}
				else
					{
					strcpy(botnick,"Trixie");
					botgender=2;
					respawntoggle=2;
					strcpy(mmapfile,"Trixie.memory");
					return(0);
					}
				}
			else
				{
				
				if(CIgrepitw(ts3,"Jeeves"))
					sayit("Oh faithful Jeeves, we feel so lost without you.  Couldn't you grace us with your presence!",sock);
				else
					sayit("Oh beautiful Trixie, our lives are meaningless without your presence.  Please come forth and take your throne by my side, and we shall reign together for all time!",sock);
				return(0);
				}
			}
		}
	if(CIgrepitw(ts2,"ignore"))
		{
		getobject(ts3,sent);
		if(nickdata[nickindex].opped)
			{
			if(isnick(ts3))
				{
				addmemory(ts3,"ignore",ignores);
				sprintf(s,"I am now ignoring %s",ts3);
				debugsayit(s,sock);
				}
			}
		// otherwise...  if the object is authorized, op it.
		else
			{
			sayit("I'm afraid I can't do that.",sock);
			}
		
		
		}
	}
if(quality)
	{
	char	notvalue[10];
	printf("Adding new memory.\n");
	add_thing(rootdata, subnp, objnp, vb, md);
	printf("Bug testing.  ri->thing = %d\n",rootdata->things);
	printf("Done adding new memory.\n");
	x = addmemory2(negate,subject,object,memories);
	if(negate)
		strcpy(notvalue,"not ");
	else
		strcpy(notvalue,"");
	if(x == 1)
		sprintf(s,"I will now remember that %s is %s%s.",subject,notvalue,object);
	else if(x == 2)
		sprintf(s,"Dude!!! I already KNEW that!!!");
	else if(x == 3)
		{
		if(!negate)
			strcpy(notvalue,"not ");
		else
			strcpy(notvalue,"");
		sprintf(s,"That's strange.  Someone told me that %s was %s%s.  I'll make a note",subject,notvalue,object);
		}
	else if(x == 4)
		{
		freesentence(&sent,NULL);
		return(1);
		}
	else
		printf("We shouldn't get here. x=%d\n",x);
	debugsayit(s,sock);
	}
if(possession)
	{
	char	notvalue[10];
	printf("doing posessions.\n");
	x = addmemory2(negate,subject,object,possessions);
	if(negate)
		strcpy(notvalue,"not ");
	else
		strcpy(notvalue,"");
	printf("done adding.\n");
	if(x == 1)
		sprintf(s,"I will now remember that %s has %s%s.",subject,notvalue,object);
	else if(x == 2)
		sprintf(s,"Dude!!! I already KNEW that!!!");
	else if(x == 3)
		sprintf(s,"That's strange.  Someone told me that %s had %s.  I'll forget about it then.",subject,object);
	else if(x == 4)
		{
		freesentence(&sent,NULL);
		return(1);
		}
	debugsayit(s,sock);
	}
printf("freeing sentence.\n");
freesentence(&sent,NULL);
printf("freed sentence.\n");
}


translateobject(char *buf, char *retbuf, char *lastnick, char *nick, int nickindex)
{
int	x,y,z;
char	s[1024];


if(CIgrepitw(buf,"me"))
	strcpy(retbuf,nick);
else if(CIgrepitw(buf,"him"))
	strcpy(retbuf,lastnick);
else if(CIgrepit(buf,"her"))
	strcpy(retbuf,lastnick);
else if(CIgrepitw(buf,"it"))
	strcpy(retbuf,lastnick);
else if(CIgrepitw(buf,"you"))
	{
	if(nickdata[nickindex].talkingto[0] != 0)
		strcpy(retbuf,nickdata[nickindex].talkingto);
	else
		strcpy(retbuf,lastnick);
	}
else
	strcpy(retbuf,buf);
}


int addmemory2(int negate,char *nick, char *obj, struct memory2 *memories)
{
int	x,y,z;
int	done;
char	s[50],t[50];

// First check to find a blank memory or if it already exists.
done=0;
printf("Adding memory.  nick %s   object: %s\n",nick, obj);
for(z=0;z<10;z++)
	{
	if(CIgrepitw(memories[z].nick,nick))
		{
		for(y=0;y<15;y++)
			{
			if(CIgrepitw(memories[z].objects[y],obj))
				{
				// found a match.
				if(negate == -1)
					{
					return(memories[z].sign[y]+1);
					}
				if(negate == memories[z].sign[y])
					{
//					strcpy(memories[z].objects[y],"");
					memories[z].sign[y] = !negate;
					return(3);
					}
				else
					{
					memories[z].sign[y] = !negate;
					return(2);
					}
				}
			if(memories[z].objects[y][0] == 0)
				{
				// found a blank spot
				if(negate == -1)
					return(5);
				strcpy(memories[z].objects[y],obj);
				memories[z].sign[y] = !negate;
				return(1);
				}
			}
		done=1;
		break;
		}

	if(memories[z].inuse == 0)
		{
		if(negate == -1)
			return(5);
		strcpy(memories[z].nick,nick);
		memories[z].inuse = 1;
		strcpy(memories[z].objects[0],obj);
		memories[z].sign[0] = !negate;
		return(1);
		done=1;
		break;
		}
	}
if(!done) // no available slot.  Make one
	{
	if(negate == -1)
		return(5);
	for(z=1;z<10;z++)
		{
		strcpy(memories[z-1].nick,memories[z].nick);
		memories[z-1].inuse = memories[z].inuse;
		for(y=0;y<15;y++)
			{
			strcpy(memories[z-1].objects[y],memories[z].objects[y]);
			memories[z-1].sign[y] = memories[z].sign[y];
			}
		}	
	strcpy(memories[9].nick,nick);
	memories[9].inuse = 1;
	strcpy(memories[9].objects[0],obj);
	memories[9].sign[0] = !negate;
	}
}

reveal_info(char *nick, int sock,int asked, struct memory2 *memories,struct memory2 *possessions)
{
int	x,y,z,z2;
int	found;
int	found2;
char	s[1024];

printf("Revealing stuff about %s.\n",nick);
found=0;
found2=0;
for(z=0;z<10;z++)
	{
	if(CIgrepitw(nick,memories[z].nick))	
		{
		// found the nick.  
		found = 1;
		break;
		}	
	}
for(z2=0;z2<10;z2++)
	{
	if(CIgrepitw(nick,possessions[z2].nick))	
		{
		// found the nick.  
		found2 = 1;
		break;
		}	
	}
if(!found && !found2)
	{
	printf("Got here.\n");
	if(asked)
		sprintf(s,"I don't know anything about you");
	else if(CIgrepitw(nick,botnick))
		sprintf(s,"I don't know anything about myself");
	else
		sprintf(s,"I don't know anything about %s",nick);
	sayit(s,sock);
	printf("done.\n");
	}
else
	{
	if(asked)
		sprintf(s,"I know the following about you:");
	else if(CIgrepitw(nick,botnick))
		sprintf(s,"I know the following about myself");
	else 
		sprintf(s,"I know the following about %s:",nick);
	sayit(s,sock);
	printf("Searching for details\n");
	for(y=0;found && y<15;y++)
		{
		printf("Found: %s\n",memories[z].objects[y]);
		if(strcmp(memories[z].objects[y],"") != 0)
			{
			char	notvalue[10];
			if(memories[z].sign[y])
				strcpy(notvalue,"");
			else
				strcpy(notvalue,"not ");
					
			if(asked)
				sprintf(s,"You are %s%s\n",notvalue,memories[z].objects[y]);
			else if(CIgrepitw(nick,botnick))
				sprintf(s,"I am %s%s\n",notvalue,memories[z].objects[y]);
			else
				sprintf(s,"%s is %s%s\n",nick,notvalue,memories[z].objects[y]);
			sayit(s,sock);
			}	
		}
	printf("Searching for possessions\n");
	for(y=0;found2 && y<15;y++)
		{
		printf("Found: %s\n",possessions[z2].objects[y]);
		if(strcmp(possessions[z2].objects[y],"") != 0)
			{
			if(asked)
				sprintf(s,"You have %s\n",possessions[z2].objects[y]);
			else if(CIgrepitw(nick,botnick))
				sprintf(s,"I have %s\n",possessions[z2].objects[y]);
			else
				sprintf(s,"%s has %s\n",nick,possessions[z2].objects[y]);
			sayit(s,sock);
			}	
		}
	}
}


int	similar(char	*nick1, char *nick2)
{
int	x,y,z;

// this function compares two nicks.  

// first check the size.  If they're different sizes, then its ok.
if(strlen(nick1) != strlen(nick2))
	return(0);
x = 0;
for(z=0;z<strlen(nick1);z++)
	{
	if(!matchup(nick1[z],nick2[z]))
		x++;
	}
if(x < 2)
	return(1);
else
	return(0);
}

int	oneis(char a, char b, char c, char d)
{
int	x,y,z;
x=0;
y=0;
if(a == c)
	x = 1;
if(a == d)
	x = 1;
if(b == c)
	y = 1;
if(b == d)
	y = 1;
if(x && y)
	return(1);
else
	return(0);
}

int	matchup(char a, char b)
{
int	x,y,z;
char	m1[5];
char	m2[5];
char	m3[5];
char	m4[5];

m1[0] = 'I';
m1[1] = 'l';
m1[2] = '1';
m1[3] = 'i';

m2[0] = 'S';
m2[1] = '5';

m3[0] = 'E';
m3[1] = '3';

m3[0] = '0';
m3[1] = 'O';


// check to see if the letters are similar or the same.

if(a == b)
	return(1);
if(a >= 'A' && a <= 'Z')
	{
	if((a - 'A' + 'a') == b)
		return(1);
	}
if(a >= 'a' && a <= 'z')
	{
	if((a - 'a' + 'A') == b)
		return(1);
	}
for(y=0;y<4;y++)
	for(x=0;x<3;x++)
		if(oneis(a,b,m1[x],m1[y]))
			return(1);
for(y=0;y<2;y++)
	for(x=0;x<2;x++)
		if(oneis(a,b,m2[x],m2[y]))
			return(1);
for(y=0;y<2;y++)
	for(x=0;x<2;x++)
		if(oneis(a,b,m4[x],m4[y]))
			return(1);
for(y=0;y<2;y++)
	for(x=0;x<2;x++)
		if(oneis(a,b,m3[x],m3[y]))
			return(1);
}

int	activate(char	*buf,int sock,struct lampinfo *lamps)
{
int	x,y,z;
char	s[1024];

for(z=0;z<17;z++)
	{
	if(lamps[z].type == -1)
		continue;	
	if(CIgrepit(buf,"all lamps") || CIgrepit(buf,"all the lamps"))
		{
		for(z=0;z<17;z++)
			{
			if(lamps[z].type == -1)
				continue;
			sprintf(s,"turn on the %s",lamps[z].name);
			activate(s,sock,lamps);
			}
		}
	if(CIgrepit(buf,lamps[z].name))
		{
		if(lamps[z].type == 0)
			{
			debugsayit("Your order has been accepted.  I am turning on the office lamp.",sock);
			system("/var/lib/httpd/cgi-bin/lightclient1 spaz");
			}
		else
			{
			if(!getstate(z))
				{
				sprintf(s,"Your order has been accepted.  I am turning on %s.",lamps[z].name);
				debugsayit(s,sock);
				sprintf(s,"/var/lib/httpd/cgi-bin/X10lightclient %d ON 3",lamps[z].unit);
				system(s);
				}
			else
				{
				sprintf(s,"%s is already on!",lamps[z].name);
				debugsayit(s,sock);	
				}
			}
		}
	}

}

int	deactivate(char	*buf,int sock,struct lampinfo *lamps)
{
int	x,y,z;
char	s[1024];

for(z=0;z<17;z++)
	{
	if(lamps[z].type == -1)
		continue;
	if(CIgrepit(buf,"all lamps") || CIgrepit(buf,"all the lamps"))
		{
		for(z=0;z<17;z++)
			{
			if(lamps[z].type == -1)
				continue;
			sprintf(s,"turn off the %s",lamps[z].name);
			deactivate(s,sock,lamps);
			}
		}
	if(CIgrepit(buf,lamps[z].name))
		{
		if(lamps[z].type == 0)
			{
			debugsayit("Your order has been accepted.  I am turning off the office lamp.",sock);
			system("/var/lib/httpd/cgi-bin/lightclient2 spaz");
			}
		else
			{
			printf("getstate: %d\n",getstate(z));
			if(getstate(z))
				{
				sprintf(s,"Your order has been accepted.  I am turning off %s.",lamps[z].name);
				debugsayit(s,sock);
				sprintf(s,"/var/lib/httpd/cgi-bin/X10lightclient %d OFF 3",lamps[z].unit);
				system(s);
				}
			else
				{
				sprintf(s,"%s is already off!",lamps[z].name);
				debugsayit(s,sock);	
				}
			}
		}
	}
}





int	getstate(int num)
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
int	temp;

char	blacklist[80];	/* ip address to check against blacklist */
FILE	*blackfile;

	


temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */




	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		{
		perror("opening stream socket");
		exit(1);
		}
	server.sin_family = AF_INET;
	hp = gethostbyname("doorbell");
	if(hp == (struct hostent *)0)
		{
		fprintf(stderr,"%s: unknown host", "doorbell");
		exit(2);
		}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(5201);
	
	if((connect(sock, &server, sizeof(server)), 0) < 0)
		{
		perror("connecting stream socket");
		exit(1);
		}
	strcpy(s,"STATE");
	
	
	
	if((write(sock, s, strlen(s)), 0) < 0)
		perror("writing on stream socket");
	
	if(( rval = read(sock, buf, 1024)) < 0)
		perror("Reading from stream socket");
		
	close(sock);
	return((int)(buf[num] - '0'));

}


saylampstatus(int sock, struct lampinfo *lamps)
{
int	x,y,z;
char	s[1024];
char	stats[2][5];
char	plur[2][5];

strcpy(plur[0],"is");
strcpy(plur[1],"are");

strcpy(stats[0],"off");
strcpy(stats[1],"on");

for(z=0;z<17;z++)
	{
	if(lamps[z].type == -1)
		continue;
	y=strlen(lamps[z].name);
	if(lamps[z].name[y-1] == 's')
		x=1;
	else
		x=0;
	sprintf(s,"%s %s %s.",lamps[z].name,plur[x],stats[getstate(z)]);
	sayit(s,sock);
	}
}



int	pingtest(char	*host,int sock)
{
int	x,y,z;
char	s[1024];

sprintf(s,"ping -c 1 %s",host);
x = system(s);
if(x)
	{
	sprintf(s,"I'm sorry, but %s appears to be dead.",host);
	sayit(s,sock);	
	}
else
	{
	sprintf(s,"%s is alive.",host);
	sayit(s,sock);
	}
}

int	isindc(char *word)
{
int	x,y,z;
int	numverbs;
char	verbs[50][40];

strcpy(verbs[0],"am");
strcpy(verbs[1],"are");
strcpy(verbs[2],"is");
numverbs=3;

for(z=0;z<numverbs;z++)
	{
	if(CIgrepitw(word,verbs[z]))
		return(1);
	}
return(0);
}

int	issub(char *word)
{
int	x,y,z;
int	numverbs;
char	verbs[50][40];

strcpy(verbs[0],"I");
strcpy(verbs[1],"you");
strcpy(verbs[2],"he");
strcpy(verbs[3],"she");
strcpy(verbs[4],"it");
numverbs=5;

for(z=0;z<numverbs;z++)
	{
	if(CIgrepitw(word,verbs[z]))
		return(1);
	}
return(0);
}

int	isposv(char *word)
{
int	x,y,z;
int	numverbs;
char	verbs[50][40];

strcpy(verbs[0],"have");
strcpy(verbs[1],"has");
strcpy(verbs[2],"possess");
strcpy(verbs[3],"own");
strcpy(verbs[4],"possess");
numverbs=5;

for(z=0;z<numverbs;z++)
	{
	if(CIgrepitw(word,verbs[z]))
		return(1);
	}
return(0);
}

int	isverbq(char *word)
{
int	x,y,z;
int	numverbs;
char	s[1024];
char	verbs[50][40];

if(isposv(word))
	return(1);
if(isindc(word))
	return(1);

strcpy(verbs[0],"do");
strcpy(verbs[1],"does");

numverbs=2;

for(z=0;z<numverbs;z++)
	{
	if(CIgrepitw(word,verbs[z]))
		return(1);
	}

return(0);
}

/*
This function checks to see if a word is a nick.  If none of the nicks
in the channel or previously in the channel match, it then checks to see
if any nick's aliases match.  
*/

int	isnick(char *word)
{
int	x,y,z;
for(z=0;z<100;z++)
	{
	if(CIgrepitw(nickdata[z].nick,word))
		{
		return(1);
		}
	}

}


int	isverb(char *word)
{
int	x,y,z;
int	numverbs;
char	s[1024];
char	verbs[50][40];

strcpy(verbs[0],"turn");
strcpy(verbs[1],"tell");
strcpy(verbs[2],"hush");
strcpy(verbs[3],"speak");
strcpy(verbs[4],"flirt");
strcpy(verbs[5],"ignore");
strcpy(verbs[6],"quote");
strcpy(verbs[7],"spawn");
strcpy(verbs[8],"die");
strcpy(verbs[9],"respawn");
strcpy(verbs[10],"op");
strcpy(verbs[11],"assume");
strcpy(verbs[12],"kill");
strcpy(verbs[13],"list");


numverbs=14;

for(z=0;z<numverbs;z++)
	{
	if(CIgrepitw(word,verbs[z]))
		return(1);
	}

return(0);
}

int	isnoun(char *word)
{
int	x,y,z;
char	s[1024];

if(word[0] == 0)
	return(0);
sprintf(s,"/usr/bin/dict -h inferno -d wn %s > /home/pjm/dictoutput.txt",word);
system(s);
//printf("extracting.\n");
x = extract_def(1,2,0);
printf("def: %d\n",x);
if(x == 4)
	return(1);
return(0);
}

int	ifgreeting(char *word)
{
int	x,y,z;
int	numgreets;
char	greets[10][40];

strcpy(greets[0],"hello");
strcpy(greets[1],"hi");
strcpy(greets[2],"yo");
strcpy(greets[3],"hey");
strcpy(greets[4],"greetings");
strcpy(greets[5],"welcome");
strcpy(greets[6],"hola");
numgreets=7;

for(z=0;z<numgreets;z++)
	{
	if(CIgrepitw(greets[z],word))
		return(1);
	}
return(0);
}


int	isadv(char *word)
{
int	x,y,z;
char	s[1024];

/*
//printf("Isadverb.\n");
sprintf(s,"/usr/bin/dict -h inferno -d wn %s > /home/pjm/dictoutput.txt",word);
system(s);
//printf("extracting.\n");
x = extract_def(1,2,0);
printf("def: %d\n",x);
if(x == 5)
	return(1);
*/
if(CIgrepit(word,"how"))
	return(1);
return(0);
}

int	ispunc(char *word)
{
int	x,y,z;
if(word[0] == '.')
	return(1);
if(word[0] == '!')
	return(1);
if(word[0] == '?')
	return(2);
if(word[0] == ',')
	return(3);
if(word[0] == ':')
	return(3);
if(word[0] == 0)
	return(1);
return(0);
}

int	isgreeting(char *word)
{
int	x,y,z;
int	num;
char	greets[20][40];

strcpy(greets[0],"hello");
strcpy(greets[1],"hey");
strcpy(greets[2],"yo");
strcpy(greets[3],"hola");
strcpy(greets[4],"greetings");
num=5;
for(z=0;z<num;z++)
	{
	if(CIgrepitw(greets[z],word))	
		return(1);
	}
return(0);
}

int	isadj(char *word)
{
int	x,y,z;
int	numverbs;
char	s[1024];
char	verbs[50][40];

strcpy(verbs[0],"a");
strcpy(verbs[1],"an");
strcpy(verbs[2],"the");
strcpy(verbs[3],"my");
strcpy(verbs[4],"her");
strcpy(verbs[5],"his");
numverbs=6;

printf("Adj search: %s.\n",word);
for(z=0;z<numverbs;z++)
	{
	if(CIgrepitw(word,verbs[z]))
		return(1);
	}

x = atoi(word);
printf("compare: .%d. .%s.\n",x,word);
sprintf(s,"%d",x);
if(strcmp(s,word) == 0)
	return(1);


sprintf(s,"/usr/bin/dict -h inferno -d wn %s > /home/pjm/dictoutput.txt",word);
system(s);
//printf("extracting.\n");
x = extract_def(1,2,0);
//printf("def: %d\n",x);
if(x == 3)
	return(1);


return(0);
}


int	comparephrase(int pos, char	*phrase,char	words[30][40])
{
int	x,y,z;
char	wds[10][40];
// 
for(z=0;z<10;z++)
	{
	wds[z][0] = 0;
	}
sscanf(phrase,"%s %s %s %s %s %s",wds[0],wds[1],wds[2],wds[3],wds[4],wds[5]);
for(z=0;z<6 && wds[z][0] != 0; z++)
	{
	if(!CIgrepitw(wds[z],words[z+pos]))
		return(0);
	}
return(1);

}


int	suckwords(char *buf, char words[30][40])
{
int	x,y,z;
int	wordnum;
int	pos;
int	size;
int	retval;

retval=0;
printf("Suckwords: .%s.\n",buf);
pos=0;
wordnum=0;
for(z=0;z<30;z++)
	words[z][0] = 0;
size=strlen(buf);
x = 0;
while(1)
	{
	if(wordnum > 29)
		break;
	if((unsigned char)buf[x] >= 128)
		{
		retval=10;
		if((unsigned char)buf[x] == 163)
			buf[x] = 'L';
		else if((unsigned char)buf[x] == 229) 
			buf[x] = 'a';
		else if((unsigned char)buf[x] == 252) 
			buf[x] = 'u';
		else if((unsigned char)buf[x] == 206) 
			buf[x] = 'i';
		else if((unsigned char)buf[x] == 238) 
			buf[x] = 'i';
		else if((unsigned char)buf[x] == 241) 
			buf[x] = 'n';
		else if((unsigned char)buf[x] == 216) 
			buf[x] = 'O';
		else if((unsigned char)buf[x] == 208) 
			buf[x] = 'D';
		else if((unsigned char)buf[x] == 246)
			buf[x] = 'o';
		else if((unsigned char)buf[x] == 194)
			buf[x] = 'A';
		else if((unsigned char)buf[x] == 213)
			buf[x] = 'O';
		else if((unsigned char)buf[x] == 203)
			buf[x] = 'E';
		else if((unsigned char)buf[x] == 215)
			buf[x] = 'x';
		else if((unsigned char)buf[x] == 199)
			buf[x] = 'c';
		else if((unsigned char)buf[x] == 174)
			buf[x] = 'R';
		else if((unsigned char)buf[x] == 167)
			buf[x] = 's';
		else if((unsigned char)buf[x] == 223)
			buf[x] = 'b';
		else if((unsigned char)buf[x] == 181)
			buf[x] = 'u';
		else if((unsigned char)buf[x] == 131)
			buf[x] = 'F';
		else if((unsigned char)buf[x] == 135)
			buf[x] = 'F';
		else if((unsigned char)buf[x] == 134)
			buf[x] = 'T';
		else if((unsigned char)buf[x] == 195)
			buf[x] = 'A';
		else if((unsigned char)buf[x] == 192)
			buf[x] = 'A';
		else if((unsigned char)buf[x] == 209)
			buf[x] = 'N';
		else	
			{
			for(y=x;buf[y] != 0;y++)
				buf[y] = buf[y+1];
			printf("x = %d  c = .%c. b = .%d.\n",x,buf[x],(unsigned char)buf[x]);
			}
		continue;
		}
	if(buf[x] == ']' && buf[x+1] == '[')
		{
		buf[x] = 'I';
		for(y=x+1;buf[y] != 0;y++)
			buf[y] = buf[y+1];
		retval=10;

		}
	if(buf[x] == '`' && buf[x+1] == '/' && buf[x+2] == '/')
		{
		buf[x] = 'W';
		for(y=x+1;buf[y] != 0;y++)
			buf[y] = buf[y+2];
		retval=10;
		}
	if(buf[x] == '{' || buf[x] == '}' || buf[x] == '[' || buf[x] == ']')
		buf[x] = ' ';
		
	if(buf[x] == ' ' || buf[x] == 0)
		{
		y = x-pos;
		if(y > 39) 
			y = 39;
		strncpy(words[wordnum],&buf[pos],y);
		words[wordnum][y] = 0;
		wordnum++;
		while(buf[x] == ' ')
			x++;
		if(buf[x] == 0)
			{
			strcpy(words[wordnum++],".");
			break;
			}
		pos = x;
		continue;
		}
	if(buf[x] == ',')
		{
		y = x-pos;
		if(y > 39) 
			y = 39;
		strncpy(words[wordnum],&buf[pos],y);
		words[wordnum][y] = 0;
		wordnum++;
		strcpy(words[wordnum++],",");
		x++;
		while(buf[x] == ' ')
			x++;
		if(buf[x] == 0)
			break;
		pos = x;
		continue;
		}
	if(buf[x] == ':')
		{
		y = x-pos;
		if(y > 39) 
			y = 39;
		strncpy(words[wordnum],&buf[pos],y);
		words[wordnum][y] = 0;
		wordnum++;
		strcpy(words[wordnum++],":");
		x++;
		while(buf[x] == ' ')
			x++;
		if(buf[x] == 0)
			break;
		pos = x;
		continue;
		}
	if(buf[x] == '.')
		{
		printf("Found a period.\n");
		y = x-pos;
		if(y > 39) 
			y = 39;
		printf("y = %d\n",y);
		strncpy(words[wordnum],&buf[pos],y);
		words[wordnum][y] = 0;
		printf("Word: .%s.\n",words[wordnum]);
		wordnum++;
		strcpy(words[wordnum++],".");
		break;
		}
	if(buf[x] == '!')
		{
		y = x-pos;
		if(y > 39) 
			y = 39;
		strncpy(words[wordnum],&buf[pos],y);
		words[wordnum][y] = 0;
		wordnum++;
		strcpy(words[wordnum++],"!");
		break;
		}
	if(buf[x] == '?')
		{
		y = x-pos;
		if(y > 39) 
			y = 39;
		strncpy(words[wordnum],&buf[pos],y);
		words[wordnum][y] = 0;
		wordnum++;
		strcpy(words[wordnum++],"?");
		break;
		}
	x++;
	}
printf("Suckwords ended. .%s. .%s. .%s.\n",words[0],words[1],words[2]);
return(retval);
}


int	expletive_handler(int	*pos,char words[30][40],struct sentence *sent)
{
int	x,y,z;
char	s[1024];

printf("eh: pos = %d\n",*pos);
if(ispunc(words[*pos]) != 3 && ispunc(words[*pos]))
	return(0);
if(ispunc(words[*pos+1]))
	{
	if(isnick(words[*pos]))
		{
		strcpy(sent->expletive,words[*pos]);
		*pos = *pos + 1;
		if(ispunc(words[*pos]) != 3 && ispunc(words[*pos]))
			{
			printf("Ex Handler return.  pos: %d  word: %s  ip: %d\n",*pos,words[*pos],ispunc(words[*pos]));
			return(0);
			}
		*pos = *pos + 1;
		}
	}
return(1);


}


int	diagram(char	*buf, char *lastques, struct	sentence  *sent)
{
int	x,y,z;
int	sl;
int	lastword;
int	pos;
int	ques;
struct	noun	*nounptr;
struct	adjective *adj;
struct	sentence	sent2;
char	s[1024],t[1024];
char	words[30][40];


while(1) {
printf("Beginning diagram.\n");


/*
for(x=0;x<strlen(buf);x++)
	{
	if(buf[x] == '.' || buf[x] == '?' || buf[x] == '!')
		buf[x] = 0;
	}
*/

for(x=0;x<10;x++)
	words[x][0] = 0;
pos=0;
ques=0;
lastword=0;

// Stage 1:  3 word sentence. or 4 words with not
/*
sscanf(buf,"%s %s %s %s %s %s %s %s %s %s",words[0],words[1],words[2],words[3],words[4],words[5],words[6],words[7],words[8],words[9]);
*/

suckwords(buf,words);
while(words[lastword][0] != 0) 
	lastword++;
for(pos=0;words[pos][0] != 0;pos++)
	{
	if(CIgrepitw(words[pos],"I'm") || CIgrepitw(words[pos],"im"))
		{
		for(z=lastword;z > (pos+1);z--)
			{
			strcpy(words[z],words[z-1]);
			}
		strcpy(words[pos],"I");
		strcpy(words[pos+1],"am");
		}
	if(CIgrepitw(words[pos],"You're"))
		{
		for(z=lastword;z > (pos+1);z--)
			{
			strcpy(words[z],words[z-1]);
			}
		strcpy(words[pos],"you");
		strcpy(words[pos+1],"are");
		}
	if(CIgrepitw(words[pos],"he's"))
		{
		for(z=lastword;z > (pos+1);z--)
			{
			strcpy(words[z],words[z-1]);
			}
		strcpy(words[pos],"he");
		strcpy(words[pos+1],"is");
		}
	if(CIgrepitw(words[pos],"she's"))
		{
		for(z=lastword;z > (pos+1);z--)
			{
			strcpy(words[z],words[z-1]);
			}
		strcpy(words[pos],"she");
		strcpy(words[pos+1],"is");
		}
	if(CIgrepitw(words[pos],"who's"))
		{
		for(z=lastword;z > (pos+1);z--)
			{
			strcpy(words[z],words[z-1]);
			}
		strcpy(words[pos],"who");
		strcpy(words[pos+1],"is");
		}
	if(CIgrepitw(words[pos],"doesn't"))
		{
		for(z=lastword;z > (pos+1);z--)
			{
			strcpy(words[z],words[z-1]);
			}
		strcpy(words[pos],"does");
		strcpy(words[pos+1],"not");
		}
	if(CIgrepitw(words[pos],"don't"))
		{
		for(z=lastword;z > (pos+1);z--)
			{
			strcpy(words[z],words[z-1]);
			}
		strcpy(words[pos],"do");
		strcpy(words[pos+1],"not");
		}
	}
pos=0;
printf("Sentence: %s\n",buf);
printf("Words: ");
for(x=0;x<10;x++)
	if(words[x][0] != 0)
		printf(".%s. ",words[x]);
printf("\n");

// First check for a nick.  If the first word is a nick, that implies that


if(isgreeting(words[pos]))
	{
	strcpy(sent->greeting,words[pos]);
	pos++;
	}

x = expletive_handler(&pos,words,sent);
if(x == 0)
	{
	printf("Returning after expletive handler.\n");
	return(0);
	}

if(isgreeting(words[pos]))
	{
	strcpy(sent->greeting,words[pos]);
	pos++;
	}
if(ispunc(words[pos]) && ispunc(words[pos]) != 3)
	{
	printf("Returning after greeting.\n");
	return(0);
	}
/*
if(ispunc(words[pos+1]))
	{
	if(isnick(words[pos]))
		{
		strcpy(sent->expletive,words[pos]);
		pos++;
		if(ispunc(words[pos]) != 3)
			return(0);
		pos++;
		}
	}
*/

//search for a greeting


//search for a command.  The verb will be first.  Spaztest will be the 
//understood subject. object will be the recipient of the info.  
//a prep phrase will follow.  object of that phrase will be for the info.


//search for a question about state (form of be).  Check for isindc to see
//if its one of those verbs.  noun after verb is the subject.  The rest
//follows as normal.  Return as a question.  parse will then compare 
//and return yes/no.
printf("Checking .%s. for yes/no.  Next: .%s.\n",words[pos],words[pos+1]);
if(CIgrepitw(words[pos],"Yes") && ispunc(words[pos+1]))
	{
	printf("Last question: %s\n",lastques);
	strcpy(buf,lastques);
	continue;
	}
if(CIgrepitw(words[pos],"No") && ispunc(words[pos+1]))
	{
	printf("Rediagramming last question.\n");
	strcpy(buf,lastques);
	initsentence(&sent2);
	diagram(buf,lastques,&sent2);
//	printf("Sent2 Subj: %s\n",sent2.sub.subnoun.word);
	memcpy(sent,&sent2,sizeof sent2);
//	printf("Sent Subj: %s\n",sent->sub.subnoun.word);
	strcpy(sent->pred.predverb.adv.word,"not");
	buildsent(s,*sent);
	printf("Rebuilt sentence: %s\n",s);
	return(0);	
	}

if(isindc(words[pos])) 
	{
	printf("Got a question.\n");
	// asking a yes/no question
	ques=1;
		
	strcpy(sent->pred.predverb.word[0],words[pos++]);
//	strcpy(sent->pred.predverb.adv.word,"");

	z=0;
	printf("pos: %d: %s\n",pos,words[pos]);
	sent->sub.nounphrase = getnounphrase(&pos,words);

	}
else if(isverbq(words[pos])) 
	{
	int order;
	order=0;
	printf("Got a different verb.\n");
	// asking a yes/no question
	ques=1;
	if(CIgrepitw(words[pos],"do") || CIgrepitw(words[pos],"does"))
		{
		// do first, the sub, then verb.  Ignore do.
		order=1;
		pos++;
		}	
	if(order==0)
		{
		if(ispunc(words[pos]))
			return(-1);
		strcpy(sent->pred.predverb.word[0],words[pos++]);
//		strcpy(sent->pred.predverb.adv.word,"");
		}

	z=0;
	printf("pos: %d: %s\n",pos,words[pos]);
	sent->sub.nounphrase = getnounphrase(&pos,words);
	if(order==1)
		{
		if(ispunc(words[pos]))
			return(-1);
		strcpy(sent->pred.predverb.word[0],words[pos++]);
//		strcpy(sent->pred.predverb.adv.word,"");
		}
	}
else if(isverb(words[pos]))
	{
	printf("Found a verb.\n");
	// this is a command.
	strcpy(sent->pred.predverb.word,words[pos]);
	sent->sub.nounphrase = initnoun(NULL);	
	if(sent->expletive[0] != 0)
		strcpy(sent->sub.nounphrase->word,sent->expletive);
	else
		strcpy(sent->sub.nounphrase->word,"you");
	pos++;
	if(isprep(words[pos]))
		{
		sent->pred.predverb.pp = getprepphrase(&pos,words);
		}
	else
		{
		struct	noun	*np;
		printf("About to get object. pos=%d word=%s\n",pos,words[pos]);
		np = getnounphrase(&pos,words);
		if(isprep(words[pos]))
			{
			sent->pred.predverb.pp = initprep(NULL);
			strcpy(sent->pred.predverb.pp->prep,words[pos]);	
			sent->pred.predverb.pp->nounphrase = np;	
			pos++;
			}
		else
			{
			sent->pred.predobject.nounphrase = np;
			}
		}
	return(0);	

	}
else
	{
/*
Normally, the first nounphrase will be the subject. However, it can also
be the object if its a question.  Until we determine that, we have to hold
it until we get the verb phrase.
*/

	struct	noun	*np;
	printf("pos: %d: %s\n",pos,words[pos]);

//	sent->sub.nounphrase = getnounphrase(&pos,words);
	np = getnounphrase(&pos,words);
//	sent->sub.nounphrase = np;
	printf("pos: %d\n",pos);


	//assume the second word is the verb	

	if(CIgrepit(words[pos],"do") || CIgrepit(words[pos],"does"))
		{
		pos++;
		if(words[pos][0] == 0)
			{
			// someone answered a question.  Replace buf and rsrt.
			strcpy(buf,"");
			adj = sent->sub.nounphrase->adj_;
			while(adj != NULL)
				{
				strcat(buf,adj->word);
				strcat(buf," ");
				}
			strcat(buf,sent->sub.nounphrase->word);
			strcat(buf," ");
			strcat(buf,lastques);
			continue;
			}
		else if(CIgrepit(words[pos],"not"))
			{
			strcpy(sent->pred.predverb.adv.word,words[pos]);
			pos++;
			}
		else if(isverbq(words[pos]))
			{
			}
		
		else 
			{
			struct	noun	*np2;
			np2 = getnounphrase(&pos,words);
			sent->sub.nounphrase = np2;	
			sent->pred.predobject.nounphrase = np;
			strcpy(sent->pred.predverb.word[0],words[pos++]);
			if(isprep(words[pos]))
				{
				sent->pred.predverb.pp = getprepphrase(&pos,words);
				}
			return(1);
			
			}
		
		}
	else
		sent->sub.nounphrase = np;
	if(isindc(words[pos]) && ispunc(words[pos+1]))
		{

		
		// someone answered a question.  Replace buf and rsrt.
			
		diagram(lastques,lastques, sent);
		if(sent->sub.nounphrase != NULL)
			{
			if(CIgrepitw(sent->sub.nounphrase->word,"who"))
				{
				freenp(sent->sub.nounphrase,NULL);
				sent->sub.nounphrase = np;	
				if(isindc(sent->pred.predverb.word[0]))
					{
					return(0);	
					}
				else
					return(-1);
				}
			else
				return(-1);
			}
		else
			return(-1);
		cpnoun(sent->sub.nounphrase,NULL);	
		
/*
		strcpy(buf,"");
		adj = sent->sub.nounphrase->adj_;
		while(adj != NULL)
			{
			strcat(buf,adj->word);
			strcat(buf," ");
			}
		strcat(buf,sent->sub.nounphrase->word);
		strcat(buf," ");
		strcat(buf,lastques);
		continue;
*/
		}	
	if(ispunc(words[pos]))
		return(-1);
	strcpy(sent->pred.predverb.word[0],words[pos++]);
	if(isprep(words[pos]))
		{
		sent->pred.predverb.pp = getprepphrase(&pos,words);
		}
	}

//assume the third word is the object
if(CIgrepit(words[pos],"not"))
	{
	strcpy(sent->pred.predverb.adv.word,words[pos]);
	pos++;
	}

printf("currently on %d: %s\n",pos,words[pos]);
z=0;

sent->pred.predobject.nounphrase = getnounphrase(&pos,words);


if(words[pos+1][0] != 0)
	{
	// too many words.  We return -1 to tell the other side to abort.
	return(-1);
	}
if(ques)
	{
	return(1);
	}
return(0);
} }


getsubj(char	*sub, struct	sentence sent)
{
int	x,y,z;
struct	adjective *adj;
char	spacer[10][10];

strcpy(sub,"");
if(sent.sub.nounphrase == NULL)
	return(0);
adj = sent.sub.nounphrase->adj_;
while(adj != NULL)
	{
	strcat(sub,adj->word);
	strcat(sub," ");
	adj = adj->adj;
	}
strcat(sub,sent.sub.nounphrase->word);
}

getverb(char	*vb, struct sentence sent)
{
int	x,y,z;
strcpy(vb,sent.pred.predverb.word[0]);
}

getadverb(char	*adv, struct sentence sent)
{
int	x,y,z;
strcpy(adv,sent.pred.predverb.adv.word);
}

updateobject(char *nick, char *lastnick, struct sentence sent)
{
int	x,y,z;
int	found;
struct	adjective *adj;
char	spacer[10][10];

if(sent.pred.predobject.nounphrase == NULL)
	return(0);
adj = sent.pred.predobject.nounphrase->adj_;

while(adj != NULL)
	{
	found=0;
	if(CIgrepit(adj->word,"my"))
		{
		sprintf(adj->word,"%s's",nick);
		adj = adj->adj;
		continue;
		}
	if(CIgrepit(adj->word,"your"))
		{
		found=1;
		}
	if(CIgrepit(adj->word,"his"))
		found=1;
	if(CIgrepit(adj->word,"her") && sent.pred.predobject.nounphrase->word[0] != 0)
		found=1;
	if(found)
		{	
		sprintf(adj->word,"%s's",lastnick);
		}
	adj = adj->adj;
	}
}

getnp(char *obj, struct	noun *np)
{
int	x,y,z;
struct	adjective *adj;

strcpy(obj,"");
adj = np->adj_;
while(adj != NULL)
	{
	strcat(obj,adj->word);
	strcat(obj," ");
	adj = adj->adj;
	}
strcat(obj,np->word);
}

getobject(char *obj, struct sentence sent)
{
int	x,y,z;
struct	adjective *adj;
struct	adverb *adv;

strcpy(obj,"");
if(sent.pred.predobject.nounphrase == NULL)
	return(0);
adj = sent.pred.predobject.nounphrase->adj_;
while(adj != NULL)
	{
	adv = adj->adv;
	while(adv != NULL)
		{
		strcat(obj,adv->word);
		strcat(obj," ");
		adv=adv->adv;	
		}
	strcat(obj,adj->word);
	strcat(obj," ");
	adj = adj->adj;
	}
strcat(obj,sent.pred.predobject.nounphrase->word);
}

getprep(char	*word, struct	prepphrase *pp)
{
int	x,y,z;
if(pp == NULL)
	{
	word[0] = 0;
	return(0);
	}
strcpy(word,pp->prep);
}

getprepobject(char *obj, struct prepphrase *pp)
{
int	x,y,z;
struct	adjective *adj;

getnp(obj,pp->nounphrase);
}



past_tense(char	*word, char *tense)
{
int	x,y,z;

sprintf(tense,"%sed",word);
}

plural(char *word, char *plur)
{
int	x,y,z;

sprintf(plur,"%ss",word);
}

detect_tense(char *word, char *tense, char *root)
{

}

struct	adjective *initadj(char *md2)
{
int	x,y,z;
struct	adjective *adj;

if(md2 == NULL)
	adj = malloc(sizeof *adj);
else
	adj = mmalloc(md2,sizeof *adj);
adj->word[0] = 0;
adj->adj = NULL;
adj->adv = NULL;
return(adj);
}

struct	noun	*initnoun(char *md2)
{
int	x,y,z;
struct	noun	*n;
if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
n->word[0] = 0;
n->adj_ = NULL;
n->nextnoun = NULL;
return(n);
}

struct	prepphrase	*initprep(char *md2)
{
int	x,y,z;
struct	prepphrase	*n;
if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
n->prep[0] = 0;
n->nounphrase = NULL;
n->pp = NULL;
return(n);
}


struct adverb *cpadv(struct adverb *o,char *md2)
{
struct adverb *n;

if(o == NULL)
	return(NULL);
if(md2 == NULL)
	n = malloc (sizeof *n);
else
	n = mmalloc (md2,sizeof *n);
strcpy(n->word,o->word);
n->adv = cpadv(o->adv,md2);
return(n);
}


struct adjective *cpadj(struct adjective *adj,char *md2)
{
struct	adjective *n;

if(adj == NULL)
	return(NULL);

if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
strcpy(n->word,adj->word);
n->adj = cpadj(adj->adj,md2);
n->adv = cpadv(adj->adv,md2);
return(n);
}

freeverb(struct verb *o, char *md2)
{
int	x,y,z;
if(o == NULL)
	return(0);
if(md2 == NULL)
	free(o);
else
	mfree(md2,o);
}

struct verb *cpverb(struct verb *o, char *md2)
{
int	x,y,z;
struct	verb *n;

if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
strcpy(n->word[0],o->word[0]);
strcpy(n->word[1],o->word[1]);
strcpy(n->word[2],o->word[2]);

// This needs to be fixed in ALL verbs to make it a pointer.

strcpy(n->adv.word,o->adv.word);
n->adv.adv = NULL;
return(n);
}

struct noun *cpnoun(struct noun *o,char *md2)
{
int	x,y,z;
struct	noun	*n;
struct	adjective	*nadj;
struct	adjective	*oadj;

//printf("In cpnoun.\n");
if(o == NULL)
	return(NULL);
if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
strcpy(n->word,o->word);
n->adj_ = cpadj(o->adj_,md2);
return(n);
}

struct	prepphrase	*cpprep(struct	prepphrase *o,char *md2)
{
int	x,y,z;
struct	prepphrase	*n;
if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
strcpy(n->prep,o->prep);
n->nounphrase = cpnoun(o->nounphrase,md2);
n->pp = cpprep(o->pp,md2);
return(n);
}

struct	adverb *addadv_adv(struct adverb *root, char *word, char *md2)
{
struct	adverb *adv;
struct	adverb *o;

if(md2 == NULL)
	adv = malloc(sizeof *adv);
else
	adv = mmalloc(md2,sizeof *adv);
strcpy(adv->word, word);
adv->adv = NULL;
if(root->adv == NULL)
	root->adv = adv;
else
	{
	o = root->adv;
	while(o->adv != NULL)
		{
		o = o->adv;
		}
	o->adv = adv;	
	}	
return(adv);
}	

struct	adverb *addadv_adj(struct adjective *adj, char *word, char *md2)
{
struct	adverb *adv;
struct	adverb *o;
if(md2 == NULL)
	adv = malloc(sizeof *adv);
else
	adv = mmalloc(md2,sizeof *adv);
strcpy(adv->word, word);
adv->adv = NULL;
if(adj->adv == NULL)
	adj->adv = adv;
else
	{
	o = adj->adv;
	while(o->adv != NULL)
		{
		o = o->adv;
		}
	o->adv = adv;	
	}	
return(adv);
}	


struct	adjective *addadj_adj(struct adjective *n,char *word, char *md2)
{
struct	adjective	*adj;
struct	adjective	*o;

if(md2 == NULL)
	adj = malloc(sizeof *adj);
else
	adj = mmalloc(md2,sizeof *adj);
strcpy(adj->word,word);
adj->adj=NULL;
adj->adv=NULL;

if(n->adj == NULL)
	n->adj = adj;
else
	{
	o = n->adj;
	while(o->adj != NULL)
		{
		o = o->adj;
		}
	o->adj = adj;
	}
return(adj);
}

struct	adjective *addadj_n(struct	noun *n,char *word, struct adjective *prevadj, char *md2)
{
struct	adjective	*adj;
struct	adjective	*o;

if(prevadj == NULL)
	{
	if(md2 == NULL)
		adj = malloc(sizeof *adj);
	else
		adj = mmalloc(md2,sizeof *adj);
	strcpy(adj->word,word);
	adj->adj=NULL;
	adj->adv=NULL;
	}
else
	adj = prevadj;
if(n->adj_ == NULL)
	n->adj_ = adj;
else
	{
	o = n->adj_;
	while(o->adj != NULL)
		{
		o = o->adj;
		}
	o->adj = adj;
	}
return(adj);
}


int	addnick(char	*nick)
{
int	x,y,z;
struct	object_structure	*os;
struct	noun	*np;

np = initnoun(NULL);
strcpy(np->word,nick);
for(z=0;z<100;z++)
	{
	if(CIgrepitw(nickdata[z].nick,nick))
		{
		nickdata[z].online = 1;
		os = add_thing(rootdata, np,NULL, NULL, md);
		os->nick = &nickdata[z];
		freenp(np,NULL);
		return(z);
		}
	if(nickdata[z].nick[0] == 0)
		{
		strcpy(nickdata[z].nick,nick);
		strcpy(nickdata[z].talkingto,"");
		nickdata[z].opped=0;
		nickdata[z].gender=0;
		nickdata[z].karma=0;
		nickdata[z].reputation=0;
		nickdata[z].online = 1;
		os = add_thing(rootdata, np,NULL, NULL, md);
		os->nick = &nickdata[z];
		freenp(np,NULL);
		return(z);
		}
		
	}
printf("We're full.  \n");
return(-1);
}

/*
Is this word a preposition?
*/

int	isprep(char *word)
{
int	x,y,z;

printf("Testing prep for: %s\n",word);

for(z=0;z<200;z++)
	{
	if(preplist[z][0] == 0)
		break;
	if(CIgrepitw(preplist[z],word))
		{
		printf("Found it.\n");
		return(1);
		}
	}
return(0);
}


/*
Change of function here.  We initialize a noun.  Then if we find an adverb,
we initialize an adjective to hold it.  Then add to that adjective.  When
the adjective is reached, we plug the values, then add it to the noun
structure.
*/



struct	noun	*getnounphrase(int *posi, char words[30][40])
{
int	x,y,z;
int	pos;
struct	noun	*nounptr;
struct	noun	*holdnounptr;
struct	adjective *adj;
struct	adverb adv_modifier;
struct	adjective adj_modifier;


pos=*posi;
if(words[pos][0] == 0)
	return(NULL);
nounptr = initnoun(NULL);
holdnounptr = initnoun(NULL);
adv_modifier.adv = NULL;
//adj_modifier.adj = NULL;

while(1)
	{
	if(isnick(words[pos]))
		break;
	if(isadv(words[pos]))
		{
		addadv_adv(&adv_modifier,words[pos],NULL);
		printf("ADVERB!!!!!: %s\n",words[pos]);
		sleep(1);
		pos++;
		continue;
	 	}
	if((isadj(words[pos]) || isnoun(words[pos+1])) && !issub(words[pos]) && !isnick(words[pos+1]) && !isverbq(words[pos+1]))
		{
		adj = addadj_n(holdnounptr,words[pos],NULL,NULL);
		printf("Adjective: %s\n",words[pos]);
		adj->adv = adv_modifier.adv;
		adv_modifier.adv = NULL;
		pos++;
		continue;
		}
	// Not an adj, adj, or stacked noun.  
	break;
	}
nounptr->adj_ = holdnounptr->adj_;
if(words[pos][0] != '.' && words[pos][0] != '!' && words[pos][0] != '?')
	{
	printf("Noun: %s\n",words[pos]);
	strcpy(nounptr->word,words[pos]);
	pos++;
	}
*posi=pos;
adj = nounptr->adj_;
printf("nounphrase: ");
while(adj != NULL)
	{
	printf("%s ",adj->word);
	adj = adj->adj;	
	}
printf("%s\n",nounptr->word);
return(nounptr);
}

struct	noun	*oldgetnounphrase(int *posi, char words[30][40])
{
int	x,y,z;
int	pos;
struct	noun	*nounptr;
struct	adjective *adj;

pos=*posi;
if(words[pos][0] == 0)
	return(NULL);
nounptr = initnoun(NULL);
while( (isadj(words[pos]) || isnoun(words[pos+1]) ) && !issub(words[pos]) && !isnick(words[pos+1]))
	{
	printf("%s (%d) is an adjective.\n",words[pos],pos);
	addadj_n(nounptr,words[pos],NULL,NULL);
	pos++;
	}
if(words[pos][0] != '.' && words[pos][0] != '!' && words[pos][0] != '?')
	{
	printf("Noun: %s\n",words[pos]);
	strcpy(nounptr->word,words[pos]);
	}
pos++;
*posi=pos;
return(nounptr);
}

struct	prepphrase	*getprepphrase(int *posi, char words[30][40])
{
int	x,y,z;
int	pos;
struct	noun	*nounptr;
struct	prepphrase	*prepptr;

pos=*posi;
if(words[pos][0] == 0)
	return(NULL);

prepptr = initprep(NULL);
if(words[pos][0] != '.' && words[pos][0] != '!' && words[pos][0] != '?')
	{
	strcpy(prepptr->prep,words[pos]);
	printf("Prep: %s\n",prepptr->prep);
	}
else
	return(NULL);
pos++;

prepptr->nounphrase = getnounphrase(&pos, words);
*posi=pos;
return(prepptr);
}

int	malename(char	*nick)
{
int	x,y,z;
int	numnames;
char	names[50][40];
strcpy(names[0],"Restil");
strcpy(names[1],"oops");
strcpy(names[2],"rho");
strcpy(names[3],"wpsnumpy");
strcpy(names[4],"Sean");
strcpy(names[5],"Jeeves");
numnames = 6;
for(z=0;z<numnames;z++)
	{
	if(CIgrepitw(names[z],nick))
		return(1);
	} 
return(0);
}

int	femalename(char	*nick)
{
int	x,y,z;
int	numnames;
char	names[50][40];
strcpy(names[0],"Jessica");
strcpy(names[1],"Nikita");
strcpy(names[2],"Tammy");
strcpy(names[3],"Leslie");
strcpy(names[4],"Celestina");
strcpy(names[5],"Sexkitten");
strcpy(names[6],"Spazure");
strcpy(names[7],"LiLxoxAngelxo");
strcpy(names[8],"devilish");
strcpy(names[9],"Trixie");
strcpy(names[10],"Duchess");
strcpy(names[11],"bunny");
strcpy(names[12],"Aimee");
strcpy(names[13],"Emily");
strcpy(names[14],"wickedbecca");
strcpy(names[15],"|Queen|");
strcpy(names[16],"Slashchick");
strcpy(names[17],"Corky");
numnames = 18;
for(z=0;z<numnames;z++)
	{
	if(CIgrepitw(names[z],nick))
		return(1);
	} 
return(0);
}

int	malegreeting(int sock, char *nick)
{
int	x,y,z;
char	s[1024];

ran = rand();
pick = (int)((4.0*ran)/(RAND_MAX+1.0));
switch(pick)
	{
	case 0: sprintf(s,"Hey stud."); break;
	case 1: sprintf(s,"Hey sexy!"); break;
	case 2: sprintf(s,"Hey cutie."); break;
	case 3: sprintf(s,"I've always wanted a man just like %s, but I've never been able..  oh SHHH!!  He's here!",nick); break;
	}
sayit(s,sock);
		
}


int	femalegreeting(int sock, char *nick)
{
int	x,y,z;
char	s[1024];

ran = rand();
pick = (int)((7.0*ran)/(RAND_MAX+1.0));
switch(pick)
	{
	case 0: sprintf(s,"Hey beautiful."); break;
	case 1: sprintf(s,"Hey sexy!"); break;
	case 2: sprintf(s,"Hey cutie."); break;
	case 3: sprintf(s,"Well hello young beautiful one!."); break;
	case 4: sprintf(s,"HEY EVERYBODY!!!!  Its %s!!!!!!.",nick); break;
	case 5: sprintf(s,"Your Royal Highness, %s has arrived!",nick); break;
	case 6: sprintf(s,"And as I was saying, %s has to be the most beautiful woman I have ever...   SHHH everyone, she's here!",nick); break;
	}
sayit(s,sock);
		
}

int	normalgreeting(int sock, char *tmpnick)
{
int	x,y,z;
char	s[1024];
ran = rand();
printf("Rand: %d\n",ran);
pick = (int)((6.0*ran)/(RAND_MAX+1.0));
switch(pick)
	{
	case 0: 
		sprintf(s,"Welcome %s\n",tmpnick);
		break;
	case 1: 
		sprintf(s,"Hello %s\n",tmpnick);
		break;
	case 2: 
		sprintf(s,"Hey %s\n",tmpnick);
		break;
	case 3: 
		sprintf(s,"Yo %s!\n",tmpnick);
		break;
	case 4: sprintf(s,"Hi %s.\n",tmpnick);
		break;	
	case 5: sprintf(s,"Hola %s.\n",tmpnick);
		break;	
	}	
sayit(s,sock);
if(CIgrepit(tmpnick,"Anonymous_User"))
	{
	sprintf(s,"%s, Since you have not yet assigned a name for yourself, you can change your nickname by typing /nick NEWNAME    where NEWNAME is the name you want.",tmpnick);
	sayit(s,sock);
	}
}

lampdiagram()
{
int	x,y,z;
int	pos;
char	words[30][40];

for(z=0;z<17;z++)
	{
	pos=0;
	if(lamps[z].name[0] == 0)
		continue;
	printf("Diagramming: %s\n",lamps[z].name);
	for(y=0;y<30;y++)
		{
		words[y][0] = 0;
		}
	sscanf(lamps[z].name,"%s %s %s %s %s %s %s %s %s %s",words[0],words[1],words[2],words[3],words[4],words[5],words[6],words[7],words[8],words[9]);
	lamps[z].nounphrase = getnounphrase(&pos, words);
	}
}


int	buildpp(char *buf, struct prepphrase *pp)
{
int	x,y,z;
char	s[1024];
strcpy(buf,"");
if(pp == NULL)
	return(0);
strcat(buf,pp->prep);
strcat(buf," ");
buildnp(s,pp->nounphrase);
strcat(buf,s);
}


int	buildnp(char *buf, struct noun *np)
{
int	x,y,z;
char	s[1024];
struct	adjective *adj;
struct	adverb	*adv;

strcpy(buf,"");
if(np == NULL)
	return(0);
adj = np->adj_;
while(adj != NULL)
	{
	adv = adj->adv;
	while(adv != NULL)
		{
		strcat(buf,adv->word);
		strcat(buf," ");
		adv = adv->adv;
		}
	strcat(buf,adj->word);
	strcat(buf," ");
	adj = adj->adj;
	}
strcat(buf,np->word);
}


int	buildsent(char *buf,struct sentence sent)
{
int	x,y,z;
char	s[1024];

strcpy(buf,"");
buildnp(s,sent.sub.nounphrase);
strcat(buf,s);
strcat(buf," ");
strcat(buf,sent.pred.predverb.word[0]);
strcat(buf," ");
if(sent.pred.predverb.adv.word[0] != 0)
	{
	strcat(buf,sent.pred.predverb.adv.word);
	strcat(buf," ");
	}
buildpp(s,sent.pred.predverb.pp);
if(s[0] != 0)
	{
	strcat(buf,s);
	strcat(buf," ");
	}
buildnp(s,sent.pred.predobject.nounphrase);
if(s[0] != 0)
	{
	strcat(buf,s);
	}
}

struct	sentence *cpsent(struct	sentence *sent, char *md2)
{
int	x,y,z;
struct	sentence *newsent;

if(md2 == NULL)
	newsent = malloc(sizeof *newsent);
else
	newsent = mmalloc(md2,sizeof *newsent);

initsentence(newsent);
strcpy(newsent->expletive,sent->expletive);
strcpy(newsent->greeting,sent->greeting);
newsent->sub.nounphrase = cpnoun(sent->sub.nounphrase,md2);
newsent->pred.predobject.nounphrase = cpnoun(sent->pred.predobject.nounphrase,md2);
newsent->pred.indirectobject.nounphrase = cpnoun(sent->pred.predobject.nounphrase,md2);
newsent->pred.comp = NULL;
strcpy(newsent->pred.predverb.word[0],sent->pred.predverb.word[0]);
strcpy(newsent->pred.predverb.word[1],sent->pred.predverb.word[1]);
strcpy(newsent->pred.predverb.word[2],sent->pred.predverb.word[2]);
strcpy(newsent->pred.predverb.adv.word,sent->pred.predverb.adv.word);
newsent->pred.predverb.adv.adv = cpadv(sent->pred.predverb.adv.adv,md2);
newsent->pred.predverb.pp = cpprep(sent->pred.predverb.pp,md2);
return(newsent);
}

initsentence(struct sentence *sent)
{
int	x,y,z;

sent->expletive[0] = 0;
sent->greeting[0] = 0;
sent->sub.nounphrase = NULL;
sent->pred.predobject.nounphrase = NULL;
sent->pred.indirectobject.nounphrase = NULL;
sent->pred.comp = NULL;
sent->pred.predverb.word[0][0] = 0;
sent->pred.predverb.word[1][0] = 0;
sent->pred.predverb.word[2][0] = 0;
sent->pred.predverb.adv.word[0] = 0;
sent->pred.predverb.adv.adv = NULL;
sent->pred.predverb.pp = NULL;
}

freethinglist(struct obj_list *o, char *md2)
{
int	x,y,z;
if(o == NULL)
	return(0);
freethinglist(o->nextobj,md2);
if(md2 == NULL)
	free(o);
else
	mfree(md2,o);
}


struct	obj_list *cpthinglist(struct obj_list *o, char *md2)
{
int	x,y,z;
struct	obj_list *n;
if(o == NULL)
	return(NULL);
if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
n->obj = o->obj;
n->nextobj = cpthinglist(o->nextobj,md2);
return(n);
}

struct	verb_list *cpvl(struct verb_list *vl, char *md2)
{
int	x,y,z;
struct	verb_list *n;

if(vl == NULL)
	return(NULL);
if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
n->vb = cpverb(vl->vb,md2);
n->tense = vl->tense;
n->next_verb = cpvl(vl->next_verb,md2);
n->objlist = cpthinglist(vl->objlist,md2);
return(n);
}

init_thing(struct object_structure *os)
{
int	x,y,z;

os->nounphrase = NULL;
os->next_obj = NULL;
os->next_np = NULL;
os->nick = NULL;
os->reverse_vl = NULL;
os->forward_vl = NULL;
}

struct	object_structure *cpthing(struct object_structure *os,char *md2)
{
int	x,y,z;
struct 	object_structure	*n;

//printf("Bug test.  noun = %s\n",os->nounphrase->word);
//printf("I'm in cpthing.\n");
if(os == NULL)
	return(NULL);
//printf("About to malloc.\n");
if(md2 == NULL)
	n = malloc(sizeof *n);
else
	n = mmalloc(md2,sizeof *n);
//printf("Done mallocing. n=%d.\n",n);

init_thing(n);
//printf("Done initing. n = %d  n->nounphrase = %d\n",n,n->nounphrase);
//printf("Bug test.  noun = %s\n",os->nounphrase->word);
n->nounphrase = cpnoun(os->nounphrase,md2);
//printf("Copy noun done.\n");
n->nick = os->nick;
//printf("Copy nick done.\n");
n->reverse_vl = cpvl(os->reverse_vl,md2);
n->forward_vl = cpvl(os->forward_vl,md2);
//printf("Done.\n");
return(n);
}


struct object_structure *insert_osnp(struct object_structure *osroot, struct object_structure *os, char *md2)
{
int	x,y,z;

//printf("I'm in the insert_osnp function.\n");
//printf("osroot = %d   os = %d\n",osroot, os);
if(comparenp2(osroot->nounphrase, os->nounphrase))
	{
//	printf("op1= %d  op2 = %d\n",osroot->nounphrase, os->nounphrase);
//	printf("Branch1\n");
	return(osroot);
	}
else
	{
//	printf("Branch2\n");
	if(osroot->next_np == NULL)
		{
		osroot->next_np = cpthing(os,md2);
		return(osroot->next_np);
		}
	else
		return(insert_osnp(osroot->next_np,os,md2));
	}
	

}

int	compareadj(struct adjective *adj1, struct adjective *adj2)
{
int	x,y,z;

if(adj1 == adj2 && adj2 == NULL)
	return(1);
if(adj1 == NULL || adj2 == NULL)
	return(0);
if(!CIgrepitw(adj1->word,adj2->word))
	return(0);
if(!compareadj(adj1->adj, adj2->adj))
	return(0);
return(1);

}


int	comparenp1(struct noun *np1, struct noun *np2)
{
int	x,y,z;

if(np1 == np2 && np2 == NULL)
	return(1);
//printf("First.\n");
if(np1 == NULL || np2 == NULL)
	return(0);

if(!CIgrepitw(np1->word, np2->word))
	return(0);
return(1);
}

int	comparenp2(struct noun *np1, struct noun *np2)
{
int	x,y,z;

//printf("comparing. np1 = %d  np2 = %d\n",np1, np2);

if(np1 == np2 && np2 == NULL)
	return(1);
//printf("First.\n");
if(np1 == NULL || np2 == NULL)
	return(0);
//printf("Second.\n");

//printf("About to compare nouns.\n");
if(!CIgrepitw(np1->word, np2->word))
	return(0);
//printf("About to compare adjectives.\n");
if(!compareadj(np1->adj_, np2->adj_))
	return(0);
return(1);
}


struct object_structure *insert_os(struct object_structure *osroot, struct object_structure *os, char *md2)
{
int	x,y,z;

//printf("Bug test:  os noun: %s\n",os->nounphrase->word);
if(osroot == NULL)
	{
	osroot = cpthing(os,md2);
	return(osroot);
	}

if(comparenp1(osroot->nounphrase, os->nounphrase))
	{
//	printf("branch 1. onp = %d\n",osroot->nounphrase);
	return(insert_osnp(osroot,os,md2));
	}
else
	{
//	printf("Branch 2.\n");
	if(osroot->next_obj == NULL)
		{
		osroot->next_obj = cpthing(os,md2);
		return(osroot->next_obj);
		}
	else
		return(insert_os(osroot->next_obj,os,md2));
	}
return(osroot);
}


int	compareverb(struct verb *vb1, struct verb *vb2)
{
int	x,y,z;

z = 1;
if(!CIgrepitw(vb1->word[0],vb2->word[0]))
	return(0);
if(!CIgrepitw(vb1->adv.word,vb2->adv.word))
	return(0);
return(1);
}

struct	verb_list *add_vl(struct verb_list *rootvl, struct verb_list *vl, char *md2)
{
int	x,y,z;
if(rootvl == NULL)
	{
	rootvl = cpvl(vl,md2);
	return(rootvl);
	}
if(compareverb(rootvl->vb,vl->vb))
	{
	return(rootvl);
	}	
else
	{
	if(rootvl->next_verb == NULL)
		{
		rootvl->next_verb = cpvl(vl,md2);
		return(rootvl->next_verb);
		}
	else
		return(add_vl(rootvl->next_verb,vl,md2));
	}
}

struct	obj_list *add_obj_list(struct obj_list *ol, struct object_structure *os, char *md2)
{
int	x,y,z;
struct	obj_list *tmpol;

if(ol == NULL)
	{
	if(md2 == NULL)
		tmpol = malloc(sizeof *tmpol);
	else
		tmpol = mmalloc(md2,sizeof *tmpol);
	
	tmpol->obj = os;
	tmpol->nextobj = NULL;	
	return(tmpol);
	}
else
	{
	ol->nextobj = add_obj_list(ol->nextobj,os,md2);
	return(ol);
	}
}


struct	object_structure	*add_thing(struct rootinfo *ri,struct noun *subject, struct noun *object, struct verb *vb,char *md2)
{
int	x,y,z;
struct	object_structure *ostag1;
struct	object_structure *ostag2;
struct	object_structure *os1;
struct	object_structure *os2;
struct	verb_list *vl;
struct	verb_list *vl1;
struct	verb_list *vl2;
struct	obj_list *ol1;
struct	obj_list *ol2;

//printf("Got here. Start of add_thing\n");
os1 = malloc(sizeof *os1);
os2 = malloc(sizeof *os2);
init_thing(os1);
init_thing(os2);
os1->nounphrase = cpnoun(subject,NULL);
os2->nounphrase = cpnoun(object,NULL);

//printf("Malloced and copied os1, os2\n");
//printf("Bug test:  os1 noun: %s\n",os1->nounphrase->word);
ostag1 = insert_os(ri->things,os1,md2);
if(ri->things == NULL)
	{
	printf("This should only happen once.  ri->things is currently NULL.\n");
	printf("ostag1 is %d\n",ostag1);
	ri->things = ostag1;
	}
if(object != NULL)
	{
	ostag2 = insert_os(ri->things,os2,md2);
	printf("done with ostags\n");

	vl = malloc(sizeof *vl);

	vl->vb = cpverb(vb,md2);

	vl1 = add_vl(ostag1->forward_vl,vl,md2);
	if(ostag1->forward_vl == NULL)
		ostag1->forward_vl = vl1;

	vl2 = add_vl(ostag2->reverse_vl,vl,md2);
	if(ostag2->reverse_vl == NULL)
	ostag2->reverse_vl = vl2;
//	printf("Done with vl's\n");

	ol1 = add_obj_list(vl1->objlist,ostag2,md2);
	if(vl1->objlist == NULL)
		vl1->objlist = ol1;

	ol2 = add_obj_list(vl2->objlist,ostag1,md2);
	if(vl2->objlist == NULL)
		vl2->objlist = ol2;
	printf("Done with ols\n");
	free(vl);
	}
printf("Done with add_thing.\n");
// MUST FIX THIS LATER.  !!!!!!!
//freething(os1,NULL);
//freething(os2,NULL);
printf("done freeing.\n");
return(ostag1);
}


freevl(struct verb_list *vl, char *md2) 
{
int	x,y,z;

if(vl == NULL)
	return(0);

freeverb(vl->vb,md2);
freethinglist(vl->objlist,md2);
freevl(vl->next_verb,md2);
if(md2 == NULL)
	free(vl);
else
	mfree(md2,vl);
}

freething(struct object_structure *os, char *md2)
{
int	x,y,z;

if(os == NULL)
	return(0);
freenp(os->nounphrase,md2);
freevl(os->reverse_vl,md2);
freevl(os->forward_vl,md2);
if(md2 == NULL)
	free(os);
else
	mfree(md2,os);


}


freeadv(struct	adverb	*adv, char *md2)
{
int	x,y,z;
if(adv == NULL)
	return(0);
if(adv->adv != NULL)
	freeadv(adv->adv,md2);
if(md2 == NULL)
	free(adv);
else
	mfree(md2,adv);
}

freeadj(struct	adjective *adj, char *md2)
{
int	x,y,z;
if(adj == NULL)
	return(0);
if(adj->adj != NULL)
	freeadj(adj->adj, md2);
if(adj->adv != NULL)
	freeadv(adj->adv,md2);
if(md2 == NULL)
	free(adj);
else
	mfree(md2,adj);
}

freenp(struct	noun	*n, char *md2)
{
int	x,y,z;
if(n == NULL)
	return(0);
if(n->adj_ != NULL)
	freeadj(n->adj_,md2);
if(n->nextnoun != NULL)
	freenp(n->nextnoun, md2);
if(md2 == NULL)
	free(n);
else
	mfree(md2,n);
}

freepp(struct prepphrase *pp,char *md2)
{
int	x,y,z;
if(pp == NULL)
	return(0);
if(pp->nounphrase != NULL)
	freenp(pp->nounphrase,md2);
if(pp->pp != NULL)
	freepp(pp->pp,md2);
if(md2 == NULL)
	free(pp);
else
	mfree(md2,pp);
}

freesentence(struct sentence *sent, char *md2)
{
int	x,y,z;

if(sent == NULL)	
	return(0);
sent->expletive[0] = 0;
freenp(sent->sub.nounphrase, md2);
sent->sub.nounphrase = NULL;
freenp(sent->pred.predobject.nounphrase,md2);
sent->pred.predobject.nounphrase = NULL;
freenp(sent->pred.indirectobject.nounphrase,md2);
sent->pred.indirectobject.nounphrase = NULL;
if(sent->pred.comp != NULL)
	{
	freeadj(sent->pred.comp->adj,md2);
	if(md2 == NULL)
		free(sent->pred.comp);
	else
		mfree(md2,sent->pred.comp);
	}
sent->pred.comp = NULL;
sent->pred.predverb.word[0][0] = 0;
sent->pred.predverb.word[1][0] = 0;
sent->pred.predverb.word[2][0] = 0;
if(sent->pred.predverb.adv.adv != NULL)
	freeadv(sent->pred.predverb.adv.adv,md2);
sent->pred.predverb.adv.adv = NULL;
if(sent->pred.predverb.pp != NULL)
	sent->pred.predverb.pp = NULL;
}


int	getnumber(char *word, int *number)
{
int	x,y,z;

*number = -1;
if(atoi(word) != 0)
	*number = atoi(word);
if(strcmp(word,"0") == 0)
	*number = 0;
if(CIgrepitw("zero",word))
	*number = 0;
if(CIgrepitw("one",word))
	*number = 1;
if(CIgrepitw("two",word))
	*number = 2;
if(CIgrepitw("three",word))
	*number = 3;
if(CIgrepitw("four",word))
	*number = 4;
if(CIgrepitw("five",word))
	*number = 5;
if(CIgrepitw("six",word))
	*number = 6;
if(CIgrepitw("seven",word))
	*number = 7;
if(CIgrepitw("eight",word))
	*number = 8;
if(CIgrepitw("nine",word))
	*number = 9;
if(CIgrepitw("ten",word))
	*number = 10;
if(*number == -1)
	return(0);
else
	return(1);
}

int	isnumber(char	*word)
{
int	x,y,z;
x = getnumber(word,&y);
return(x);
}

/*
This function takes a nounphrase and returns the quantifier adjective if there
is one.
*/
int	getquantifier(struct adjective *adj, char *answer)
{
int	x,y,z;
strcpy(answer, "");
if(CIgrepitw(adj->word,"a") || CIgrepitw(adj->word,"an") || CIgrepitw(adj->word,"the"))
	{
	strcpy(answer,adj->word);
	return(1);
	}
if(CIgrepitw(adj->word,"some") || CIgrepitw(adj->word,"few") || CIgrepitw(adj->word,"all"))
	{
	strcpy(answer,adj->word);
	return(1);
	}
if(isnumber(adj->word))
	{
	strcpy(answer,adj->word);
	return(1);
	}
return(0);
}


/*
This function takes as its argument a nounphrase with all its adjectives
included that describe one of the lamps.  The function then determines which
stage 1:  just find the first match and return it.
stage 2:  pick a random one.
stage 3:  expand to be more general, not just for lamps
*/

int	picklamp(int mode, struct	noun *n,char answer[10][200])
{
int	x,y,z;
int	match;
int	count;
int	total;
char	validwords[10][40];
char	temp[1024];
int	lampcheck[17];

struct	adjective *adj;
struct	adjective *lampadj;
char	s[1024];


strcpy(validwords[0],"lamp");
strcpy(validwords[1],"lamps");
strcpy(validwords[2],"light");
strcpy(validwords[3],"lights");
strcpy(validwords[4],"flower");

count=0;
for(z=0;z<10;z++)
	answer[z][0] = 0;
y=0;
for(z=0;z<5;z++)
	{
	if(CIgrepitw(validwords[z],n->word))
		y=1;	
	}
if(!y)
	return(0);
for(z=0;z<17;z++)
	lampcheck[z] = 0;
// loop through all possible lamps
for(z=0;z<17;z++)
	{
	// first check to see if there even IS a lamp here.
	if(lamps[z].name[0] == 0)
		continue;
	// ok.  We have a lamp.  
/*
	We loop through all adjectives in the phrase we were provided.
	Check to see if every word provided matches every word describing this
		lamp.  
	Ignore quantifiers for now.  
*/
	match=1;
	adj = n->adj_;
	while(adj != NULL)
		{
		printf("z: %d Picklamp: Checking %s\n",z,adj->word);
		if(!getquantifier(adj,s))
			{
			printf("Picklamp: %s is not a quantifier.\n",adj->word);
			y=0; //this will be 1 if we get a match.
			lampadj = lamps[z].nounphrase->adj_;
			while(lampadj != NULL)
				{
				printf("Picklamp: lamp phrase Checking : %s\n",lampadj->word);
				if(CIgrepitw(adj->word,lampadj->word))
					{
					printf("picklamp: found it.\n");
					y=1;
					break;
					}
				lampadj = lampadj->adj;
				}
			if(!y)
				{
				// no matches found
				match = 0;
				printf("picklamp: No matches.\n");
				break;
				}
				
			}
		adj = adj->adj;
		}
	if(match)
		{
		count++;
		lampcheck[z] = 1;
		}
	}
adj = n->adj_;
total = -1;
while(adj != NULL)
	{
	if(getquantifier(adj,s))
		{
		if(isnumber(s))
			{
			getnumber(s,&total);			
			}
		if(CIgrepitw(s,"some"))
			{
			// some is more than one and less than the total.
			if(count <= 2)
				total = count;
			else
				{
				ran = rand();
				pick = (int)(((float)(count-2)*ran)/(RAND_MAX+1.0));
				total = pick + 1;
				}
			break;
			}
		if(CIgrepitw(s,"few"))
			{
			// more than one, no more than 3.	
			if(count <= 2)
				total = count;
			else
				{
				ran = rand();
				pick = (int)((2.0*ran)/(RAND_MAX+1.0));
				total = pick + 2;
				}
			break;
			}
		if(CIgrepitw(s,"all"))
			{
			total=8;
			break;
			}
		}
	adj=adj->adj;
	}
if(total == -1)
	total=1;
printf("total: %d Count: %d, pick: %d.\n",total,count,pick);

y=0;

for(z=0;z<17;z++)
	if(lampcheck[z] && (getstate(z) == !mode))
		{
		strcpy(answer[y++],lamps[z].name);
		printf("Picklamp: Match on: %s\n",lamps[z].name);
		}
for(z=0;z<y;z++)
	{
	ran = rand();
	pick = (int)(((float)y*ran)/(RAND_MAX+1.0));
	strcpy(temp,answer[z]);
	strcpy(answer[z], answer[pick]);
	strcpy(answer[pick],temp);	
	}
for(z=total;z<10;z++)
	strcpy(answer[z],"");
}

int	spellnumber(char *answer, int number)
{
int	x,y,z;
int	size;
int	pos;
int	place;
char	s[1024];
char	n[1024];

strcpy(answer,"");
sprintf(s,"%d",number);
size=strlen(s);
pos=0;

}

/*
The rootverb function takes a verb as o, returns the present tense 
singular first person form of the verb as n, then returns a value to identify
the tense.

1	: present
2 	: past
3	: participle
*/


int	rootverbcheck(char *o, char *n)
{
int	x,y,z;

if(isverb(o))
	{
	strcpy(n,o);
	return(1);	
	}
if(isindc(o))
	{
	strcpy(n,"am");
	return(1);
	}
if(isposv(o))
	{
	strcpy(n,"have");
	return(1);
	}
return(0);
}

int	rootverb(char	*o, char *n)
{
int	x,y,z;
char	s[1024], t[1024];

if(rootverbcheck(o,s))
	{
	strcpy(n,s);
	return(1);
	}

x = strlen(o);
y = x-2;
if(y > 0)
	{
	if(CIgrepitw(&o[y],"ed"))
		{
		strncpy(s,o,y);
		s[y] = 0;
		if(rootverbcheck(s,t))
			{
			strcpy(n,t);
			return(2);
			}
		
		}
	}
return(0);

}

/*
This is mostly a debug script.  It will start at the rootinfo and go through
the list of objects, printing the name. 
*/

int	listobjects(int	sock)
{
int	x,y,z;
char	t[1024],s[1024];
struct	object_structure *os;
struct	object_structure *os2;

printf("Listobject Got here.  rootdata = %d\n",rootdata);
os = rootdata->things;
printf("os = %d\n",os);
x = 0;
while(os != NULL)
	{
	y=0;
	getnp(s,os->nounphrase);
	sprintf(t,"Object #%d:  %s",x,s);	
//	sayit(t,sock);
	printf("%s\n",t);
	os2 = os->next_np;	
	while(os2 != NULL)
		{
		getnp(s,os2->nounphrase);
		sprintf(t,"Object #%d, sub #%d:  %s",x,y,s);	
		//sayit(t,sock);
		printf("%s\n",t);
		os2 = os2->next_np;
		y++;
		}
	os = os->next_obj;
	x++;
	}
}


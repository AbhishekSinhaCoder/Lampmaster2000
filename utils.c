#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>    
#include <netinet/in.h>
#include <errno.h>

#include "visitors.h"
#include "dmiheaders.h"

/*
int     getrandom(int   r)
{
int     x,y,z;
int	pick,ran;

ran = rand();
pick = (int)(((float)r*ran)/(RAND_MAX+1.0));
return(pick);
}
*/

int	max(int	a, int b)
{
if(a > b)
	return(a);
else
	return(b);
}

unsigned char hexvalue(char c)
{
if(c >= '0' && c <= '9')
	return(c-'0');
if(c >= 'A' && c <= 'F')
	return(c-'A'+10);
if(c >= 'a' && c <= 'f')
	return(c-'a'+10);
}

int	myerror(char *errorstring)
{
int	x,y,z;
FILE	*output;

output=fopen("/tmp/myerror.txt","a+");
fprintf(output,"%s\n",errorstring);
fclose(output);
}


file_append(char *filename, char *str)
{
int     x,y,z;
FILE    *output;

output = fopen(filename,"a+");
if(output != NULL)
	{
	fprintf(output,"%s\n",str);
	fclose(output);
	}
else
	{
	printf("Couldn't open file.\n");
	}
}




int	stripnulls(char *buf, int length_buffer)
{
int	x,y,z;
int	size;
char	*newbuf;

size = length_buffer;
newbuf = malloc(length_buffer+10);
y=0;
for(z=0;z<size;z++)
	{
	if(buf[z] != 0)
		{
		newbuf[y++] = buf[z];
		}
	}
newbuf[y] = 0;
strcpy(buf,newbuf);
}




myputc(char	c, char *buffer, int httpmode)
{
int	size;
size = strlen(buffer);
if(httpmode)
	{
	buffer[size] = c;
	buffer[size+1] = 0;
	}
else
	{
	putc(c,stdout);
	}
}


filter(char *buf, char *ret)
{
int	x,y,z;
int	size;
int	pos;
int	hush;
int	gender;
int	volume;
int	speed;
int	httpmode;
char	s[20000];
char	buffer[5000];
unsigned char	c,d;
pos=0;
httpmode=1;
hush=0;
volume=0;
speed=0;
strcpy(s,buf);
size = strlen(s);
//printf("httpmode = %d. pos=%d  size=%d",httpmode,pos,size);
strcpy(buffer,"");
while(pos < size)
	{
	c = s[pos++];
	if(c == '+')
		{
		if(hush <= 0)
			myputc(' ',buffer,httpmode);
		}
	else if(c == '%')
		{
		c = s[pos++];
		x = hexvalue(c);
		c = s[pos++];
		x = x*16+hexvalue(c);
		if(x == '<')
			hush++;
		if(hush <= 0)
			myputc(x,buffer,httpmode);
		if(x == '>')
			hush--;
		}
	else
		if(hush <= 0)
			myputc(c,buffer,httpmode);
	}
//printf("Done.  pos=%d size=%d.",pos,size);
while(buffer[0] == ' ')
	{
	x = strlen(buffer);
	for(z=0;z<x;z++)
		buffer[z] = buffer[z+1];
	}
while(buffer[strlen(buffer)-1] == ' ')
	buffer[strlen(buffer)-1] = 0;
strcpy(ret,buffer);
}

http_encode(char *dest, char *src)
{
int	x,y,z;
int	size;
char	temps[2000];
y=0;
strcpy(temps,"");
size = strlen(src);
for(z=0;z<size;z++)
	{
	if(src[z] == '#')
		{
		temps[y++] = '%';
		temps[y++] = '2';
		temps[y++] = '3';
		}
	else if(src[z] == ' ')
		{
		temps[y++] = '%';
		temps[y++] = '2';
		temps[y++] = '0';
		}
	else if(src[z] == '\'')
		{
		temps[y++] = '%';
		temps[y++] = '2';
		temps[y++] = '7';
		}
	else if(src[z] == '(')
		{
		temps[y++] = '%';
		temps[y++] = '2';
		temps[y++] = '8';
		}
	else if(src[z] == ')')
		{
		temps[y++] = '%';
		temps[y++] = '2';
		temps[y++] = '9';
		}
	else
		{
		temps[y++] = src[z];
		}
	}
temps[y] = 0;
strcpy(dest,temps);
}



unsigned long convertiptodec(char *host)
{
unsigned long	x,y,z;
int	a[4];
char 	s[200];

strcpy(s,host);
y=0;
a[y++] = atoi(s);
for(z=0;z<strlen(s);z++)
	{
	if(s[z] == '.')
		{
		a[y++] = atoi(&s[z+1]);
		}
	
	}
x = 0;
for(z=0;z<4;z++)
	{
	x = x << 8;
	x = x + a[z];
	}
return(x);
}



char	*xmlparse(char *token, int count, char *buffer)
{
int	x,y,z;
int	startpos;
int	counter;
int	tokensize;
int	tokensize2;
int	size;
char	searchtoken[256];
char	searchtoken2[256];
char	*retbuf;

counter=0;
sprintf(searchtoken,"<%s>",token);
sprintf(searchtoken2,"</%s>",token);
size = strlen(buffer);
tokensize = strlen(searchtoken);
tokensize2 = strlen(searchtoken2);
for(z=0;z<size;z++)
	{
	if(strncmp(&buffer[z],searchtoken,tokensize) == 0)	// found a token
		{
		if(count > counter++)
			continue;	
		startpos = z+tokensize;
		for(y=z+1;y<size;y++)
			{
			if(strncmp(&buffer[y],searchtoken2,tokensize2) == 0)
				{
				retbuf = malloc((y-startpos) + 5);		
				strncpy(retbuf,&buffer[startpos],y-startpos);
				retbuf[y-startpos] = 0;
				return(retbuf);
				}
			}
		}
	}
return(NULL);
}


int	getconnection(int sock, struct sklink **sk)
{
int	rval;
char	host[40];
struct	sockaddr_in client;
fd_set	rfds;
struct	timeval	tv;
int	retval;
int	size;
int	msgsock;

size=sizeof (client);
/* Old way: Set up select to check the socket for data.  If none is available, 
   continue on and perform other tasks.  */	

/* New way:  check for 1 usec to see if there are any connections. If so, add
accept it and add the socket to the queue.
*/

FD_ZERO(&rfds);
FD_SET(sock,&rfds);
/* Wait 1 second for data, then proceed with other stuff */
/*
tv.tv_sec=1;
tv.tv_usec=0;	
*/
tv.tv_sec=0;
tv.tv_usec=1;
retval = select(sock+1, &rfds, NULL, NULL, &tv);
if(retval)		/* we got data */
	{
	printf("Got an incoming connection.\n");
	msgsock = accept(sock, &client,&size);
	if(msgsock == -1)
		{
		perror("accept");
		return(-1);
		}
	else 
		{
		strncpy(host,inet_ntoa(client.sin_addr),39);
		host[39] = 0;
		addsocketlink(msgsock,sk,host);
		return(1);
/*
//		address_holder = (unsigned char *) &client.sin_addr.s_addr;	
	//	bzero(buf, sizeof(buf));

		printf("About to read.\n");
		FD_ZERO(&rfds);
		FD_SET(msgsock,&rfds);
		// wait 2 seconds, then give up.	
		tv.tv_sec=2;
		tv.tv_usec=0;	
		retval = select(msgsock+1, &rfds, NULL, NULL, &tv);
		if(retval <= 0)	
			{
			close(msgsock);
			return(-1);
			rval=0;
			}
		else
			return(msgsock);
*/
		}
	}
else
	return(-1);
}

/*
the xmllink function will take an xml segment and extract all tokens into a 
linked list.
*/

struct	xmllinktype	*xmllink(char *buffer)
{
}


addsocketlink(int msgsock,struct sklink **sk, char *host)
{
struct	sklink *temp;
struct	sklink *temp2;
struct	sklink *socketlink;
long	t;

socketlink = *sk;
//printf("Adding socket. socketlink=%d sk=%d\n",socketlink,sk);
temp = malloc(sizeof *temp);
temp->msgsock = msgsock;
time(&t);
strncpy(temp->host,host,39);
temp->host[39] = 0;
temp->connecttime = t;
temp->next = NULL;
if(socketlink == NULL)
	{
	socketlink = temp;
	*sk = socketlink;
	}
else
	{
	temp2 = socketlink;
	while(temp2->next != NULL)
		temp2 = temp2->next; 
	temp2->next = temp;
	}
}


int	deletesocket(int msgsock,int mode,struct sklink **sk)
{
struct	sklink *temp;
struct	sklink *temp2;
struct	sklink *socketlink;

socketlink = *sk;

//printf("deletesocket function. socketlink=%d  sk=%d\n",socketlink,sk);
if(socketlink == NULL)
	return(-1);

temp = NULL;
temp = socketlink;

if(socketlink->msgsock == msgsock)
	{
	temp = socketlink;
	socketlink=temp->next;
	*sk = socketlink;
	free(temp);
//	printf("Leaving deletesocket\n");
	return(1);
	}	
while(temp->next != NULL)
	{
	if(temp->next->msgsock == msgsock)
		{
		temp2 = temp->next;
		temp->next = temp2->next;
		free(temp2);
//		printf("Leaving deletesocket\n");
		return(1);	
		}
	else
		temp=temp->next;
	}
}


int	numsockets(struct sklink **sk)
{
int	x,y,z;
long	t;
struct	sklink *temp;
struct	sklink *temp2;
struct	sklink *socketlink;
int	msgsock;
fd_set	rfds;
struct	timeval	tv;
int	retval;

socketlink = *sk;
x=0;
while(socketlink != NULL)
	{
	x++;
	socketlink = socketlink->next;
	}
return(x);
}

int	scansockets(struct sklink **sk, char *host)
{
int	x,y,z;
long	t;
struct	sklink *temp;
struct	sklink *temp2;
struct	sklink *socketlink;
int	msgsock;
fd_set	rfds;
struct	timeval	tv;
int	retval;

socketlink = *sk;
//printf("socketlink=%d.  sk = %d\n",socketlink,sk);
time(&t);
temp = socketlink;
while(temp != NULL)
	{
	printf("Scanning msgsock %d\n",temp->msgsock);
	if(temp->connecttime + 10 < t)
		{
//		printf("temp->connecttime= %d t=%d\n",temp->connecttime + 10,t);
		close(temp->msgsock);
		deletesocket(temp->msgsock,1,sk);
		return(0);
		}
	FD_ZERO(&rfds);
	FD_SET(temp->msgsock,&rfds);
		
	tv.tv_sec=0;
	tv.tv_usec=1;	
	retval = select(temp->msgsock + 1, &rfds, NULL, NULL, &tv);
	if(retval)		/* we got data */
		{
//		printf("Got here.  msgsock=%d\n",temp->msgsock);
		strncpy(host,temp->host,39);
                host[39] = 0;
//		printf("Got here.  msgsock=%d\n",temp->msgsock);
		return(temp->msgsock);
		}
	else
		temp = temp->next;
	}
return(-1);
}

int	newgetcam(int camnum, struct camtype *cam)
{
int	x,y,z;
int	sock;
struct	camcommandtype	camcom;
int	rval;
char	buf[20000];

camcom.camnum = camnum;
strcpy(camcom.command,"GET");
sock = connect_socket("doorbell", 5209);
write(sock,&camcom,sizeof camcom);
x=0;
// buffer overflow potential to fix later
do
	{
	rval = read(sock,&buf[x],1460);
	x = x + rval;
	}
while(rval >= 1460);
if(x != sizeof *cam)
	{
	printf("Uh oh.  buf was %d but should have been %d.\n",x,sizeof *cam);
	printf("In case we care, buffer was .%s.\n",buf);
	exit(1);
	}
else
	{
	memcpy(cam,buf,sizeof *cam);	
	}
shutdown(sock,SHUT_RDWR);
//close(sock);
}

int	getcam(int camnum, struct camtype *cam)
{
int	x,y,z;
int	handle;
int	size;
char	s[2000];
struct	camcommandtype	camcom;

//fprintf(stderr,"getcam function.  num=%d\n",camnum);
/*
camcom.camnum = camnum;
strcpy(camcom.command,"GET");
*/
//myerror("about to open");
handle = open("/tmp/webcams.dat",O_RDONLY);
size = lseek(handle,0,2) / sizeof *cam;
sprintf(s,"size=%d",size);
//myerror(s);
//fprintf(stderr,"handle=%d.  Size=%d\n",handle,size);
if(size <= camnum)
        {
//	myerror("branch 1");
        close(handle);
//	myerror("returning");
        return(-1);
        }
else
        {
//	myerror("branch 2");
        lseek(handle,camnum * sizeof *cam,0);
        read(handle,cam,sizeof *cam);
        close(handle);
	//fprintf(stderr,"Cam name: %s\n",cam->name);
	//myerror("returning");
        return(1);
        }
}

int     putcam(int camnum, struct camtype *cam)
{
int     x,y,z;
int     size;
int     handle;

handle = open("/tmp/webcams.dat",O_WRONLY);
size = lseek(handle,0,2) / sizeof *cam;
if(size <= camnum)
        {
        close(handle);
        return(-1);
        }
else
        {
        lseek(handle,camnum * sizeof *cam,0);
        write(handle,cam,sizeof *cam);
        close(handle);
        return(1);
        }
}

int     appendcam(struct camtype *cam)
{
int     x,y,z;
int     size;
int     handle;
char	s[2000];

handle = open("/tmp/webcams.dat",O_RDWR);
size = lseek(handle,0,2) / sizeof *cam;
lseek(handle,size * sizeof *cam,0);
write(handle,cam,sizeof *cam);
close(handle);
sprintf(s,"Did append successfully for %s",cam->name);
myerror(s);
return(size);
}

int     getnumcams()
{
int     x,y,z;
int     size;
int     handle;
struct  camtype cam;

handle = open("/tmp/webcams.dat",O_WRONLY);
size = lseek(handle,0,2) / sizeof cam;
close(handle);
return(size);
}

int     newgetnumcams()
{
int     x,y,z;
int     size;
int	sock;
int     handle;
struct  camtype cam;
struct	camcommandtype	camcom;
char	buf[3000];

strcpy(camcom.command,"GETCAMCOUNT");
sock = connect_socket("doorbell", 5209);
if(sock == -1)
	{
	perror("connecting: ");
	return(-1);
	}
printf("sock = %d\n",sock);
y = write(sock,&camcom,sizeof camcom);
printf("wrote %d bytes.\n",y);
y = read(sock,buf,1460);
if(y == -1)
	{
	perror("reading: ");
	return(-1);
	}
//close(sock,SHUT_RDWR);
close(sock);
if(y == sizeof x)
	{
	memcpy(&x,buf,y);
	return(x);
	}
else
	{
	printf("Serious problem.  only got %d bytes back.  Expected %d\n",y,sizeof x);
	return(-1);
	}
}



int getcookiename(char *host, char *retbuf, char *retname)
{
int	sock;
int	rval;
struct	in_addr	revname;
struct	sockaddr_in server;
struct	hostent *hp, *hp2;
char	buf[1024];
char	s[1024];
int	x,y,z;
char	c;

char 	*fname;
int 	fd, status;
int	temp;
char	cookiestr[20];
char	name[40];
char	fullhost[256];
char	starthost[256];
int	number;
int	mode; // 0 - default/normal, 1 - print name

//printf("Getcookiename.  host: .%s.\n",host);
strcpy(fullhost,"");
strcpy(starthost,host);
mode = 0;
temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */

//printf("about to do sock.\n");
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		{
		perror("opening stream socket");
		exit(1);
		}
	server.sin_family = AF_INET;
//printf("about to do gethostbyname.\n");
	hp = gethostbyname(COOKIESERVER_HOST);
//printf("did it.");
	if(hp == (struct hostent *)0)
		{
		fprintf(stderr,"%s: unknown host","doorbell");
		exit(2);
		}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(5202);
	
//printf("about to do connect.\n");
	if((connect(sock, &server, sizeof(server)), 0) < 0)
		{
		printf("Server problem.\n");
		perror("connecting stream socket");
		if(retbuf != NULL)
			strcpy(retbuf,"Cookie server is currently down.");
		if(retname != NULL)
			strcpy(retname,"Serverdown");
		return(-4);
		}
	strcpy(s,"");
	sprintf(s,"-3 # %s",host);
	fprintf(stderr,"getcookiename s: .%s.\n",s);
	
	if((write(sock, s, strlen(s)), 0) < 0)
		perror("writing on stream socket");
	
	if(( rval = read(sock, buf, 1024)) < 0)
		perror("Reading from stream socket");
		
	close(sock);
//	printf("Message: .%s.\n",buf);	
	x = atoi(buf);
//	printf("Number returned was: %d\n",x);
	if(x > 0)
		sscanf(buf,"%d %s %s",&number,name,host);
	else
		{
		number=-3;
		strcpy(host,starthost);
		sprintf(name,"Guest%u",convertiptodec(host));
		}
	if(name[0] == '#')
		sprintf(name,"Guest%u",convertiptodec(host));
	
	revname.s_addr = inet_addr(host);
	hp2 = gethostbyaddr((const char *)&revname,sizeof (struct in_addr),AF_INET);
	if(hp2 != NULL)
		strcpy(fullhost,hp2->h_name);
/*
	else
		perror("gethostbyaddr");
*/
	if(retbuf != NULL)
		sprintf(retbuf,"Number: %d  Name: %s  IP: %s  Hostname: %s",number,name,host,fullhost);
	if(retname != NULL)
		{
		strcpy(retname,name);
		}
	return(number);
}

int	getvisitorbyname(char *nick, struct visitortype *retvisitor)
{
int	x,y,z;
int	handle;
int	size;
struct	visitortype	visitor;

if(nick[0] == 0)
	return(-1);
handle = open("/var/lib/httpd/cgi-bin/userdatabase.dat",O_RDONLY);
size = lseek(handle,0,2) / sizeof visitor;
lseek(handle,0,0);
retvisitor->nick[0] = 0;
for(z=0;z<size;z++)
	{
	read(handle,&visitor, sizeof visitor);
	if(strcmp(nick,visitor.nick) == 0)
		{
		fprintf(stderr,"Found a match.\n");
		close(handle);
		memcpy(retvisitor,&visitor,sizeof visitor);
		return(z);
		}
	}
close(handle);
return(-1);
}



int	newgetvisitorname(int cookienumber,struct visitortype *retvisitor,char *username, char *password)
{
int	handle;
int	x,y,z;
int	size;
int	mode;
char	s[2000],t[2000];
char	qs[10000];
struct	visitortype	visitor;


mode=0;
if(username[0] != 0 && password[0] != 0)
	{
	mode=1;
	}
handle = open("/var/lib/httpd/cgi-bin/userdatabase.dat",O_RDWR);
size = lseek(handle,0,2) / sizeof visitor;
for(z=0;z<size;z++)
	{
	lseek(handle,z*sizeof visitor,0);
	read(handle,&visitor,sizeof visitor);
	if(mode == 0 && visitor.cookienumber == cookienumber && visitor.cookienumber != -1)
		{
		visitor.lastcheckdate = time(NULL);
		lseek(handle,z*sizeof visitor,0);
		write(handle,&visitor,sizeof visitor);
		close(handle);
		memcpy(retvisitor,&visitor,sizeof visitor);
		return(z);
		}
	if(mode == 1 && strcmp(visitor.nick,username) == 0 && strcmp(visitor.password,password) == 0)
		{
		visitor.lastcheckdate = time(NULL);
		lseek(handle,z*sizeof visitor,0);
		write(handle,&visitor,sizeof visitor);
		close(handle);
		memcpy(retvisitor,&visitor,sizeof visitor);
		return(z);
		}
	}
close(handle);
return(-1);
}

int	getvisitorname(int cookienumber,struct visitortype *retvisitor)
{
int	x,y,z;
char	username[256],password[256];
char	qs[10000];
char	s[10000];


if(getenv("QUERY_STRING") != NULL)
	strcpy(qs,getenv("QUERY_STRING"));
else
	strcpy(qs,"");
findtoken("username",qs,s);
filter(s,username);
findtoken("password",qs,password);
x = newgetvisitorname(cookienumber,retvisitor,username,password);
return(x);
}

int	display_switchdata(struct lampdisplaytype *ldt)
{
int	sock;
int	rval;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();
char	buf[1024];
char	s[1024];
int	x,y,z,w;
int	pos;
char	c;

char 	*fname;
int 	fd, status;
int	temp;



temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */




	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		{
		perror("opening stream socket");
		exit(1);
		}
	server.sin_family = AF_INET;
	hp = gethostbyname(LAMPSERVER_HOST);
	if(hp == (struct hostent *)0)
		{
		fprintf(stderr,"%s: unknown host", LAMPSERVER_HOST);
		exit(2);
		}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(5201);
	
	if((connect(sock, &server, sizeof(server)), 0) < 0)
		{
		perror("connecting stream socket");
		exit(1);
		}
	strcpy(s,"GETLAMPDATA");
	
	if((write(sock, s, strlen(s)), 0) < 0)
		perror("writing on stream socket");
	
	if(( rval = read(sock, buf, 1024)) < 0)
		perror("Reading from stream socket");
	close(sock);

y = strlen(buf);
x=0;
z=0;
w=0;
for(pos=0;pos<y;pos++)
	{
	if(buf[pos] == ';' && w==0)
		{
		strncpy(ldt[z].name,&buf[x],pos-x);
		ldt[z].name[pos-x] = 0;
		x=pos+1;
		w=1;
		continue;
		}
	if(buf[pos] == ';' && w==1)
		{
		strncpy(ldt[z].label,&buf[x],pos-x);
		ldt[z].label[pos-x] = 0;
		x = pos+1;
		w=2;
		continue;
		}
	if(buf[pos] == ';' && w==2)
		{
		strncpy(ldt[z].camname,&buf[x],pos-x);
		ldt[z].camname[pos-x] = 0;
		x = pos+1;
		w=3;
		continue;
		}
	if(buf[pos] == '\n' || buf[pos] == 0)
		{
		ldt[z].icon = atoi(&buf[x]);
		x = pos+1;
		z++;
		continue;
		}
	}
return(z);
}


int	connect_socket(char	*connecthost, int port)
{
int	sock;
int	rval;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();
char	buf[1024];
char	s[1024];
int	x,y,z;
int	pos;
char	c;

char 	*fname;
int 	fd, status;
int	temp;




temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */




	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		{
		perror("opening stream socket");
		exit(1);
		}
	server.sin_family = AF_INET;
	hp = gethostbyname(connecthost);
	if(hp == (struct hostent *)0)
		{
		fprintf(stderr,"%s: unknown host", connecthost);
		return(-1);
		}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(port);
	
	if((connect(sock, &server, sizeof(server)), 0) < 0)
		{
		perror("connecting stream socket");
		return(-1);
		}
return(sock);
}

int	timed_connect_socket(char	*connecthost, int port, int timeout)
{
int	sock;
int	rval;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();
char	buf[1024];
char	s[1024];
int	x,y,z;
int	optlen;
char	optval[2000];
int	pos;
char	c;
int	flags;

char 	*fname;
int 	fd, status;
int	temp;
fd_set	rfds;
fd_set	wfds;
struct	timeval	tv;
int	retval;





temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */




sock = socket(AF_INET, SOCK_STREAM, 0);
if(sock < 0)
	{
	perror("opening stream socket");
	exit(1);
	}
server.sin_family = AF_INET;
hp = gethostbyname(connecthost);
if(hp == (struct hostent *)0)
	{
	fprintf(stderr,"%s: unknown host", connecthost);
	return(-1);
	}
bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
server.sin_port = htons(port);

flags = fcntl(sock, F_GETFL,0);
fcntl(sock, F_SETFL, flags | O_NONBLOCK);
//fprintf(stderr,"about to connect.\n");	
z = connect(sock, &server, sizeof(server));
if(z == -1)
	{
//	fprintf(stderr,"errno=%d eip=%d\n",errno,EINPROGRESS);
	if(errno != EINPROGRESS)
		{
		perror("connect");
		return(-1);
		}
	else
		{
//		fprintf(stderr,"eip mode.\n");
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		FD_ZERO(&wfds);
		FD_SET(sock,&wfds);
		tv.tv_sec=timeout;
		tv.tv_usec=0;	
		retval = select(sock+1, NULL, &wfds, NULL, &tv);
//		fprintf(stderr,"retval=%d\n",retval);
		if(retval < 1)
			return(-1);
		else
			{
//			fprintf(stderr,"about to getsockopt.\n");	
			y = getsockopt(sock,SOL_SOCKET,SO_ERROR,optval,&optlen);
//			fprintf(stderr,"done. optlen=%d optval=%d\n",optlen,optval);
//			fprintf(stderr,"getsockopt.  y=%d x=%d\n",y,x);
			if(y < 0)
				{
				perror("getsockopt");
				return(-1);
				}
			
			if(optlen == 0)
				return(sock);
			else if((int)optval == 0)
				return(sock);
			else
				return(-1);
			
			}
		}
	}
return(sock);
}



int	read_socket(int sock, char *buffer, int numchars)
{
int	x,y,z;
int	pos;
int	rval;
int	retval;
fd_set	rfds;
struct	timeval	tv;

FD_ZERO(&rfds);
FD_SET(sock,&rfds);
		
pos=0;
tv.tv_sec=0;
tv.tv_usec=1;	
retval = select(sock+1, &rfds, NULL, NULL, &tv);
while(retval > 0)
	{
	x = 1460;
	if(pos + 1460 > numchars)	
		x = numchars - pos;
	if(x <= 0)
		break;
	if((rval = read(sock, &buffer[pos],x)) < 0)
		perror("Reading from stream socket");
	pos = pos + rval;
	retval = select(sock+1, &rfds, NULL, NULL, &tv);
	}
return(pos);
}

int	write_socket(int sock, char *buffer, int numchars)
{
int	x,y,z;
int	pos;
int	rval;
fd_set	rfds;
struct	timeval	tv;

		
pos=0;
while(pos < numchars)
	{
	x = numchars - pos;
	if(x <= 0)
		break;
	if((rval = write(sock, &buffer[pos],x)) < 0)
		perror("Writing to stream socket");
	pos = pos + rval;
	}
return(pos);
}



int	islistening(char *cookiename)
{
int	x,y,z;
int	sock;
char	s[2000];
char	t[20000];
char	name[200];
char	hosts[2000];
char	nickhost[2000];
char	host[200];
struct	hostent *hp2;

sprintf(host,"inferno");
//printf("Started.\n");
sock = connect_socket(host,8000);
if(sock == -1)
	exit(1);	
//printf("Connected.\n");
//sleep(2);
sprintf(s,"ADMIN motfm1\r\n\r\n");
write(sock,s,strlen(s));
//printf("Written\n");
//sleep(1);
z=0;
x = read(sock,t,1440);
//printf("Read first section.\n");
//printf("buf = .%s.\n",t);
//sleep(1);

sprintf(s,"list\r\n");
write(sock,s,strlen(s));
sprintf(s,"quit\r\n");
write(sock,s,strlen(s));
z=0;
//sleep(1);
x = read(sock,t,1800);
while(x > 0)
	{
	z += x;
	x = read(sock,&t[z],1800);
	}
//printf(".%s.\n",t);
z=0;
while(t[z] != 0)
	{
	if(strncmp(&t[z],"[Id:",4) == 0)
		{
		for(y=z;t[y] != 0 && t[y] != '\n';y++)
			{
			if(strncmp(&t[y],"[Host:",6) == 0)
				break;
			}
		z=y+7;
		for(y=z;t[y] != 0 && t[y] != 0 && t[y] != '\n' && t[y] != ']';y++);
		strncpy(s,&t[z],y-z);
		s[y-z] =0;
		strcpy(hosts,s);
		z=y;
		for(y=z;t[y] != 0 && t[y] != '\n';y++)
			{
			if(strncmp(&t[y],"[Type:",6) == 0)
				break;
			}
		z=y+7;
		for(y=z;t[y] != 0 && t[y] != 0 && t[y] != '\n' && t[y] != ']';y++);
		if(strncmp(&t[z],"client",6) == 0)
			{
			hp2=gethostbyname(hosts);
                        if(hp2 == NULL)
                                continue;
//                        printf("hp2 is not null.\n");
                        if(strcmp("",inet_ntoa(*(struct in_addr*)hp2->h_addr_list[0])) == 0)
                                strcpy(nickhost,hosts);
                        else
                                strcpy(nickhost,inet_ntoa(*(struct in_addr*)hp2->h_addr_list[0]));

			getcookiename(nickhost,NULL,name);
			if(name[0] != 0)
				{
				if(strcmp(name,cookiename) == 0)
					return(1);
				}
			}
		}
	else
		z++;
	}
close(sock);
return(0);
}




int	getcookienumber()
{
char	cookiestr[256];
int	number;
if(getenv("HTTP_COOKIE") == NULL)
	strcpy(cookiestr,"");
else
	strcpy(cookiestr,getenv("HTTP_COOKIE"));
if(cookiestr[0] == 0)
	number = -1;
else
	{
	number = atoi(&cookiestr[9]);
	if(number < 1)
		number = -1;
	}
return(number);
}


int	gettheme()
{
int	x,y,z;
int	handle;
int	number;
int	size;
char	s[256];
char	qs[10000];
struct	visitortype	visitor;


if(getenv("QUERY_STRING") != NULL)
	strcpy(qs,getenv("QUERY_STRING"));
else
	strcpy(qs,"");
findtoken("theme",qs,s);
if(s[0] != 0)
	{
	x = atoi(s);
	if(x >= 0 && x <= 7)
		return(x);
	}
number = getcookienumber();
if(number >= 0)
	{
	handle = open("/var/lib/httpd/cgi-bin/userdatabase.dat",O_RDONLY);
	size = lseek(handle,0,2) / sizeof visitor;
	for(z=0;z<size;z++)
		{
		lseek(handle,z*sizeof visitor,0);
		read(handle,&visitor,sizeof visitor);
		if(visitor.cookienumber == number)
			{
			close(handle);
			if(visitor.theme < 0)
				return(0);
			if(visitor.theme > 7)
				return(0);
			return(visitor.theme);
			}
		}
	}
close(handle);
return(0);
}



int	cookiecheck(int	mode,char *name)
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
char	host[200];
int	number;
FILE	*logfile;

number = getcookienumber();
fprintf(stderr,"cookie number=%d\n",number);
if(getenv("REMOTE_ADDR") == NULL)
	strcpy(host,"255.255.255.255");
else
	strcpy(host,getenv("REMOTE_ADDR"));
temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */

//logfile=fopen("/tmp/cookielog","a+");
//fprintf(logfile,"mode: %d  cookiestr: .%s.\n",mode,cookiestr);
//fclose(logfile);	
strcpy(name,"#");
fprintf(stderr,"cookie number=%d  mode=%d\n",number,mode);
if(mode == 1 && number == -1)
	{
	sprintf(name,"Guest%u",convertiptodec(host));
	return(-1);
	}


	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		{
		perror("opening stream socket");
		exit(1);
		}
	server.sin_family = AF_INET;
	hp = gethostbyname(COOKIESERVER_HOST);
	if(hp == (struct hostent *)0)
		{
		fprintf(stderr,"%s: unknown host",COOKIESERVER_HOST);
		exit(2);
		}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(5202);
	
	if((connect(sock, &server, sizeof(server)), 0) < 0)
		{
		perror("connecting stream socket");
		exit(1);
		}
	strcpy(s,"");
	sprintf(s,"%d %s %s",number,name,host);
	
	if((write(sock, s, strlen(s)), 0) < 0)
		perror("writing on stream socket");
	
	if(( rval = read(sock, buf, 1024)) < 0)
		perror("Reading from stream socket");
		
	close(sock);
//	printf("Message: .%s.\n",buf);	
	sscanf(buf,"%d %s %s",&number,name,host);
if(name[0] == '#')
	sprintf(name,"Guest%u",convertiptodec(host));
fprintf(stderr,"cookie name:%s\n",name);
return(number);
}

int	getcookiebynumber(int	number,char *name)
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
char	host[200];
FILE	*logfile;

fprintf(stderr,"cookie number=%d\n",number);
strcpy(host,"255.255.255.255");
temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */

//logfile=fopen("/tmp/cookielog","a+");
//fprintf(logfile,"mode: %d  cookiestr: .%s.\n",mode,cookiestr);
//fclose(logfile);	
strcpy(name,"#");
if(number == -1)
	{
	sprintf(name,"Guest%u",convertiptodec(host));
	return(-1);
	}


	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		{
		perror("opening stream socket");
		exit(1);
		}
	server.sin_family = AF_INET;
	hp = gethostbyname(COOKIESERVER_HOST);
	if(hp == (struct hostent *)0)
		{
		fprintf(stderr,"%s: unknown host",COOKIESERVER_HOST);
		exit(2);
		}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(5202);
	
	if((connect(sock, &server, sizeof(server)), 0) < 0)
		{
		perror("connecting stream socket");
		exit(1);
		}
	strcpy(s,"");
	sprintf(s,"%d %s %s",number,name,host);
	
	if((write(sock, s, strlen(s)), 0) < 0)
		perror("writing on stream socket");
	
	if(( rval = read(sock, buf, 1024)) < 0)
		perror("Reading from stream socket");
		
	close(sock);
//	printf("Message: .%s.\n",buf);	
	sscanf(buf,"%d %s %s",&number,name,host);
if(name[0] == '#')
	sprintf(name,"Guest%u",convertiptodec(host));
fprintf(stderr,"cookie name:%s\n",name);
return(number);
}

int	getquerystring(char	*qs)
{
if(getenv("QUERY_STRING") != NULL)
	strcpy(qs,getenv("QUERY_STRING"));
else
	strcpy(qs,"");
}

int	findtoken(char *token,char *str,char *value)
{
int	w,x,y,z;
char	tok[40];
sprintf(tok,"%s=",token);
y = strlen(tok);
z = strlen(str);
//fprintf(stderr,"Searching for .%s. in .%s.  y=%d  z=%d\n",tok,str,y,z);
for(x=0;x<z;x++)
	{
	if(strncmp(tok,&str[x],y) == 0)
		{
		if(x > 0)
			if(str[x-1] != '&')
				continue;
//		printf("Found the token.\n");
		for(w=x+y;str[w] != '&' && str[w] != 0;w++);
		strncpy(value,&str[x+y],w-(x+y));
		value[w-(x+y)] = 0;
//		printf("w=%d  x=%d  y=%d  z=%d\n",w,x,y,z);
		return(x+y);
		}
	}
strcpy(value,"");
return(-1);
}


int	findinttoken(char *token,char *qs)
{
int	x,y,z;
char	s[2000];

findtoken(token,qs,s);
if(s[0] == 0)
	x = atoi(s);
else
	x = 0;
return(x);
}


int	mystrncpy(char *dest, char *src, int size)
{
int	x,y,z;
int	dsize,ssize;

//printf("src= .%s.\n",src);
ssize = strlen(src);
if(ssize > size)
	ssize = size;
y=0;
for(z=0;z<ssize;z++)
	{
	if(src[z] == '\\' && src[z+1] == '\"')
		{
		dest[y] = '\"';
		z++;
		}
	else if(src[z] == '\\' && src[z+1] == '\\')
		{
		dest[y] = '\\';
		z++;
		}
	else
		dest[y] = src[z];
	y++;
	}
dest[y] = 0;
return(y);
}



int	gettoken(int pos, char *line, char *token)
{
int	x,y,z;
int	quotes;
int	size;

z=pos;
quotes=0;
if(line[z] != 0)
	{
	quotes=0;
	while(line[z] == ' ' || line[z] == '\"' || line[z] == '\n')
		z++;
	if(z > 0 && line[z-1] == '\"')
		quotes=1;
	x = z++;
	while(!((line[z] == ' ' || line[z] == '\n') && !quotes) && line[z] != 0 && line[z] != '\"')
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
//strncpy(token,&line[x],size);
token[size] = 0;
return(z);
}


int	getlargetoken(int pos, char *line, char *token)
{
int	x,y,z;
int	quotes;
int	size;

z=pos;
quotes=0;
if(line[z] != 0)
	{
	quotes=0;
	while(line[z] == ' ' || (line[z] == '\"' && line[z-1] != '\\'))
		z++;
	if(z > 0 && line[z-1] == '\"' && line[z-2] != '\\')
		quotes=1;
	x = z++;
	while(!(line[z] == ' ' && !quotes) && line[z] != 0 && !(line[z] == '\"' && line[z-1] != '\\'))
		z++;
	y = z;
	size = y-x;
	if(size > 20000)
		size=20000;
	}
else
	{
	size=0;
	x=0;
	}

mystrncpy(token,&line[x],size);
//strncpy(token,&line[x],size);
//token[size] = 0;
return(z);
}


initproject(struct projecttype *project)
{
int	x,y,z;
project->name[0] = 0;
project->status = 0;
project->type = 0;
project->desc[0] = 0;
project->laborhours=0;
project->rdhours=0;
project->workedhours = 0;
project->materialcost = 0;
project->hourlyrate=0;
project->submitaccount[0] = 0;
project->workaccount[0] = 0;
project->priority = 0;
project->complexity =0;
project->softwarehours=0;
project->subproject=0;
project->subprojectname[0] = 0;
}

initcam(struct camtype *camdata)
{
int	x,y,z;

camdata->scheduling = 0;
camdata->status = 0;
camdata->message_destination = 0;
strcpy(camdata->message_token,"");
camdata->show_profile = 0;
camdata->show_website = 0;
camdata->adult = 0;
camdata->adult_mode = 0;
camdata->commercial=0;
camdata->forcecache=0;
camdata->autocache=100;
camdata->cacherate=5;
camdata->audiohost[0] = 0;
camdata->audioport=0;
camdata->audioadminpass[0] = 0;
camdata->audiotype=0;
camdata->phone=0;
camdata->phoneurl[0]=0;
time(&camdata->creationdate);
camdata->lastupdate = camdata->creationdate;
camdata->lampaccount[0] = 0;
camdata->active=0;
camdata->static_addr=0;
camdata->available=0;
camdata->refresh_rate=0;
camdata->update_rate=20;
camdata->archive=0;
camdata->archive_rate = 20;
camdata->name[0] =0;
camdata->audio=0;
camdata->music=0;
camdata->localfiles=0;
strcpy(camdata->httpqhost,"");
camdata->httpqport = 0;
strcpy(camdata->audiourl,"");
for(z=0;z<10;z++)
	{
	camdata->cd[z].host[0] = 0;
	camdata->cd[z].port = 0;
	camdata->cd[z].available = 0;
	camdata->cd[z].filename[0] = 0;
	}
camdata->password[0] = 0;
camdata->type=0;
camdata->popup=1;
camdata->capture=1;
camdata->pan=0;
camdata->tilt=0;
camdata->zoom=0;
camdata->message=1;
camdata->lamps=0;
camdata->showarchive=0;
}

oldinitcam(struct camtype *camdata)
{
int	x,y,z;

camdata->active=0;
camdata->static_addr=0;
camdata->available=0;
camdata->refresh_rate=0;
camdata->update_rate=20;
camdata->archive=0;
camdata->archive_rate = 20;
camdata->name[0] =0;
camdata->audio=0;
camdata->music=0;
camdata->localfiles=0;
strcpy(camdata->httpqhost,"");
camdata->httpqport = 0;
strcpy(camdata->audiourl,"");
for(z=0;z<10;z++)
	{
	camdata->cd[z].host[0] = 0;
	camdata->cd[z].port = 0;
	camdata->cd[z].available = 0;
	camdata->cd[z].filename[0] = 0;
	}
camdata->password[0] = 0;
camdata->type=0;
camdata->popup=1;
camdata->capture=1;
camdata->pan=0;
camdata->tilt=0;
camdata->zoom=0;
camdata->message=1;
camdata->lamps=0;
camdata->showarchive=0;
}


readwebcamconfig(char *filename)
{
FILE	*in;
int	handle;
int	x,y,z;
int	mode;
int	lastmode;
int	housemode;
int	unitmode;
int	times;
int	camcount;
char	line[2000];
char	token[256];
char	value[256];
char	s[256];
struct	camtype	camdata[100];

for(z=0;z<100;z++)
	initcam(&camdata[z]);
lastmode = 0;
mode=0;
times=0;
camcount=0;


in = fopen("webcams.cfg","r");
if(in == NULL)
	{
	printf("The webcams.cfg file does not exist.  Create one.\n");
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
	if(strcmp(token,"[cam]") == 0)
		{
		lastmode = mode;
		mode = 1;
		}
	if(strcmp(token,"[ie]") == 0)
		{
		mode = 2;
		camdata[camcount].cd[mode-2].available=1;	
		}
	if(strcmp(token,"[ns]") == 0)
		{
		mode = 3;
		camdata[camcount].cd[mode-2].available=1;	
		}
	if(strcmp(token,"[asf]") == 0)
		{
		mode = 4;
		camdata[camcount].cd[mode-2].available=1;	
		}
	if(strcmp(token,"[mpg]") == 0)
		{
		mode = 5;
		camdata[camcount].cd[mode-2].available=1;	
		}
	if(strcmp(token,"[auasf]") == 0)
		{
		mode = 8;
		camdata[camcount].cd[mode-2].available=1;	
		}
	if(strcmp(token,"[screen1]") == 0)
		{
		mode = 9;
		camdata[camcount].cd[mode-2].available=1;	
		}
	if(strcmp(token,"[screen2]") == 0)
		{
		mode = 10;
		camdata[camcount].cd[mode-2].available=1;	
		}
	if(strcmp(token,"[screen3]") == 0)
		{
		mode = 11;
		camdata[camcount].cd[mode-2].available=1;	
		}
		
	if(lastmode)
		{
		camcount++;
		lastmode=0;
		}
	if(mode == 1)
		{
		if(strcmp(token,"name") == 0)
			strcpy(camdata[camcount].name,value);	
		if(strcmp(token,"account") == 0)
			strcpy(camdata[camcount].account,value);	
		if(strcmp(token,"password") == 0)
			strcpy(camdata[camcount].password,value);	
		if(strcmp(token,"active") == 0)
			camdata[camcount].active = atoi(value);	
		if(strcmp(token,"refresh_rate") == 0)
			camdata[camcount].refresh_rate = atoi(value);	
		if(strcmp(token,"update_rate") == 0)
			camdata[camcount].update_rate = atoi(value);	
		if(strcmp(token,"archive_rate") == 0)
			camdata[camcount].archive_rate = atoi(value);	
		if(strcmp(token,"type") == 0)
			camdata[camcount].type = atoi(value);	
		if(strcmp(token,"popup") == 0)
			camdata[camcount].popup = atoi(value);	
		if(strcmp(token,"capture") == 0)
			camdata[camcount].capture = atoi(value);	
		if(strcmp(token,"pan") == 0)
			camdata[camcount].pan = atoi(value);	
		if(strcmp(token,"tilt") == 0)
			camdata[camcount].tilt = atoi(value);	
		if(strcmp(token,"zoom") == 0)
			camdata[camcount].zoom = atoi(value);	
		if(strcmp(token,"message") == 0)
			camdata[camcount].message = atoi(value);	
		if(strcmp(token,"message") == 0)
			camdata[camcount].message = atoi(value);	
		if(strcmp(token,"lamps") == 0)
			camdata[camcount].lamps = atoi(value);	
		if(strcmp(token,"static") == 0)
			camdata[camcount].static_addr = atoi(value);	
		if(strcmp(token,"showarchive") == 0)
			camdata[camcount].showarchive = atoi(value);	
		if(strcmp(token,"private") == 0)
			camdata[camcount].private = atoi(value);	
		if(strcmp(token,"audio") == 0)
			camdata[camcount].audio = atoi(value);
		if(strcmp(token,"audiourl") == 0)
			strcpy(camdata[camcount].audiourl,value);
		if(strcmp(token,"httpqhost") == 0)
			strcpy(camdata[camcount].httpqhost,value);
		if(strcmp(token,"music") == 0)
			camdata[camcount].music = atoi(value);
		if(strcmp(token,"localfiles") == 0)
			camdata[camcount].localfiles = atoi(value);
		if(strcmp(token,"httpqport") == 0)
			camdata[camcount].httpqport = atoi(value);
		}
	if(mode >= 2)
		{
		if(strcmp(token,"host") == 0)
			strcpy(camdata[camcount].cd[mode-2].host,value);	
		if(strcmp(token,"port") == 0)
			camdata[camcount].cd[mode-2].port = atoi(value);
		if(strcmp(token,"filename") == 0)
			strcpy(camdata[camcount].cd[mode-2].filename,value);	
			
		}
	}				
fclose(in);
handle = open(filename,O_RDWR);
lseek(handle,0,0);
for(z=0;z<camcount+1;z++)
	{
	lseek(handle,z*sizeof camdata[0],0);
	write(handle,&camdata[z],sizeof camdata[0]);
	}
close(handle);
for(z=0;z<camcount+1;z++)
	{
	char temps[2000];
	sprintf(temps,"mkdir \"/extreme/archives/%s\"",camdata[z].name);
	system(temps);
	}
}

int	add_cookie_block(char *blockname)
{
FILE    *blockfile;
blockfile = fopen("/home/pjm/blocksc.dat","a+");
fprintf(blockfile,"%s\n",blockname);
fclose(blockfile);
}

int	add_ip_block(char *blockname)
{
FILE    *blockfile;
blockfile = fopen("/home/pjm/blocksi.dat","a+");
fprintf(blockfile,"%s\n",blockname);
fclose(blockfile);
}

int	add_cam_block(int camnum, char *blockname)
{
int	x,y,z;
int	handle;
FILE	*output;
int	size;
char	s[256];
char	tempname[80];
struct	camtype cam;

getcam(camnum,&cam);
sprintf(s,"/extreme/blocks/%s",cam.name);
output = fopen(s,"a+");
fprintf(output,"%s\n",blockname);
fclose(output);
}

int	add_private_name(char *camname, char *privatename)
{
int	x,y,z;
int	handle;
int	size;
char	s[256];
char	tempname[80];

sprintf(s,"/extreme/privacy/%s",camname);
handle = open(s,O_WRONLY);
if(handle == -1)
	{
	handle = open(s,O_WRONLY | O_CREAT, S_IRWXU);
	}
size = lseek(handle,0,2) / sizeof tempname;
lseek(handle,size * sizeof tempname,0);
write(handle,privatename,sizeof tempname);
close(handle);
}

int	add_cam_schedule(int camnum, struct camschedtype camsched)
{
int	x,y,z;
int	size;
int	handle;
char	s[256];
char	tempname[80];
struct	camtype cam;


getcam(camnum,&cam);
sprintf(s,"/extreme/cam_schedules/%s",cam.name);
handle = open(s,O_WRONLY);
if(handle == -1)
	{
	handle = open(s,O_WRONLY | O_CREAT, S_IRWXU);
	}
size = lseek(handle,0,2) / sizeof camsched;
lseek(handle,size * sizeof camsched,0);
write(handle,&camsched,sizeof camsched);
close(handle);
}

int	delete_private_name(char *camname, char *privatename)
{
int	x,y,z;
int	handle,handle2;
int	size;
char	s[256];
char	tempname[80];

sprintf(s,"/extreme/privacy/%s",camname);
handle = open(s,O_RDWR);
if(handle == -1)
	{
	return(-1);
	}
sprintf(s,"/extreme/privacy/temp%s",camname);
handle2 = open(s,O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU );
if(handle2 == -1)
	{
	return(-1);
	}
size = lseek(handle,0,2) / sizeof tempname;
lseek(handle,0,0);
lseek(handle2,0,0);
for(z=0;z<size;z++)
	{
	read(handle,tempname,sizeof tempname);
	//lseek(handle2,z * sizeof tempname,0);
	if(strcmp(tempname,privatename) == 0)
		{
		continue;
		}
	write(handle2,tempname,sizeof tempname);
	}
/*
if(z < size)
	{
	for(y=z+1;y < size;y++)
		{
		lseek(handle,y * sizeof tempname,0);
		read(handle,tempname,sizeof tempname);
		lseek(handle2,(y-1) * sizeof tempname,0);
		write(handle2,tempname,sizeof tempname);
		}
	}
*/
close(handle);
close(handle2);
sprintf(s,"mv \"/extreme/privacy/temp%s\" \"/extreme/privacy/%s\"",camname,camname);
system(s);
}

readfaqconfig()
{
FILE	*in;
FILE	*output;
int	handle;
int	x,y,z;
int	mode;
int	lastmode;
int	housemode;
int	unitmode;
int	times;
int	faqcount;
char	line[25000];
char	token[256];
char	value[20000];
char	s[25000];

struct	faqtype	
	{
	char	*question;
	char	*answer;
	char	*catagory;
	}faqdata[200];

lastmode = 0;
mode=0;
times=0;
faqcount=0;

printf("I'm here.\n");
in = fopen("/var/lib/httpd/htdocs/data/faqdata.txt","r");
if(in == NULL)
	{
	printf("The faqdata.txt file does not exist.  Create one.\n");
	exit(1);
	}
while (!feof(in))
	{
	// read a line from the config file.
	line[0] = 0;
	fgets(line, 24990, in);
	line[24990] = 0;
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
	y = getlargetoken(x,line,value);
	if(strcmp(token,"") == 0)
		continue;
	if(strcmp(token,"[faq]") == 0)
		{
		lastmode = mode;
		mode = 1;
		}
	if(lastmode)
		{
		faqcount++;
		lastmode=0;
		}
	if(mode == 1)
		{
		if(strcmp(token,"question") == 0)
			{
			//printf("Got a question: .%s.\n",value);
			faqdata[faqcount].question = malloc(strlen(value)+5);
			strcpy(faqdata[faqcount].question,value);
			}
		if(strcmp(token,"answer") == 0)
			{
			//printf("Got an answer: .%s.\n",value);
			faqdata[faqcount].answer = malloc(strlen(value)+5);
			strcpy(faqdata[faqcount].answer,value);
			}
		if(strcmp(token,"catagory") == 0)
			{
			//printf("Got a catagory: .%s.\n",value);
			faqdata[faqcount].catagory = malloc(strlen(value)+5);
			strcpy(faqdata[faqcount].catagory,value);
			}

		}
	}				
fclose(in);
printf("Done reading.\n");
output = fopen("/var/lib/httpd/htdocs/newfaq.shtml","w+");
fprintf(output,"<html><head><title>Faq</title></head>\n");
fprintf(output,"<!--#exec cmd=\"/var/lib/httpd/cgi-bin/printresponse \\\"FAQ\\\" 5\"-->\n");
fprintf(output,"<!--#exec cmd=\"/var/lib/httpd/cgi-bin/printresponse \\\"FAQ\\\"\"-->\n");
fprintf(output,"<P>\n");
for(z=0;z<faqcount+1;z++)
	{
	fprintf(output,"<P>\n");
	fprintf(output,"(%d) <A NAME=\"F%d\" href=\"#F%d\"><strong><font size=+1 color=\"Blue\">%s</font></strong></A><P><P>\n",z,z,z,faqdata[z].question);
	fprintf(output,"%s<P><P><hr>\n",faqdata[z].answer);
	}
fprintf(output,"<!--#exec cmd=\"/var/lib/httpd/cgi-bin/printresponse \\\"FAQ\\\" 1\"-->\n");
fprintf(output,"</body>\n");
fprintf(output,"</html>\n");
fclose(output);
printf("Done.\n");
}


int	newisprivatecam(int cam, char *accountname)
{
int	x,y,z;
int	xxx;
int	size;
int	cookienum;
int	handle;
int	matchcookie;
int	matchip;
int	matchhost;
int	matchaccount;
char	s[2000],t[2000];
struct	camtype	cams;
struct	visitortype visitor;

matchaccount=matchip=matchhost=matchcookie=0;
matchip=matchhost=1; // we're going to match ip addresses and hosts always
size = getnumcams();
if(cam >= size)
	{
	return(0);
	}
getcam(cam,&cams);
if(cams.private)
	{
	if(accountname[0] == 0)
		return(1);
/*
	cookienum = cookienumber;
	if(cookienum == -1)
		{
		// no valid cookie, so no matching accounts or cookies.
		matchaccount=0;
		matchcookie=0;
		}
	else
		{
		matchcookie=1;
		matchaccount=1;
		getvisitorname(cookienum, &visitor);
		if(strcmp(visitor.nick,"Restil") == 0)
			return(0);      // Restil can view all private cams.
		if(strcmp(visitor.nick,"Gertie") == 0)
			return(0);      // Gertie can view all private cams.
		}
*/
	y = getvisitorbyname(accountname,&visitor);
	if(y == -1)
		return(1);
	if(strcmp(visitor.nick,"Restil") == 0)
		return(0);      // Restil can view all private cams.
	if(strcmp(visitor.nick,"Gertie") == 0)
		return(0);      // Gertie can view all private cams.
	sprintf(s,"/extreme/privacy/%s",cams.name);
	handle=open(s,O_RDONLY);
	if(handle == -1)
		{
		//  there is no private list, private for everyone.
		return(1);
		}
	size = lseek(handle,0,2) / 80;
	for(xxx = 0; xxx < size; xxx++)
		{
		lseek(handle,xxx * 80,0);
		read(handle,s,80);
		if(s[0] == 0)
			continue;
		if(strncmp(s,"IP:",3) == 0)	// ip address
			{
			if(getenv("REMOTE_ADDR") != NULL)
				strcpy(t,getenv("REMOTE_ADDR"));
			else
				strcpy(t,"");
			if(strcmp(&s[3],t) == 0 && t[0] != 0)
				{
				close(handle);
			
				fprintf(stderr,"isprivate2: not private. s:%s t:%s\n",s,t,visitor.nick);
				return(0);	
				}
			}
		else if(matchaccount && strcmp(s,visitor.nick) == 0)	// match accounts
			{
			close(handle);
			fprintf(stderr,"isprivate: not private. s:%s  vis:%s\n",s,visitor.nick);
			return(0);
			}
		}
	return(1);	
	}
else
	return(0);
}

int	isprivatecam(int camnum)
{
int	x,y,z;
int	cookienum;
struct	visitortype	visitor;
cookienum = getcookienumber();
getvisitorname(cookienum, &visitor);
x = newisprivatecam(camnum,visitor.nick);
return(x);
}

printstate(char	*buf, int cam, char *cookiename)
{
int	x,y,z;
int	handle;
char	URL[256];
char	windowname[256];
char	s[256];
long	lt;
struct	lampdisplaytype ldt[100];
struct	camtype	cams;
int	inum;


findtoken("strike",buf,s);
z=atoi(s);
inum=display_switchdata(ldt);
time(&lt);
getcam(cam,&cams);
y = ipblocker() | cookieblocker() | percamblocker(cams.name,cookiename); 
if(y)
	z = -1;
/*
if(cam > -1)
	{
	if(isprivatecam(cam))
		{
		z = -1;		
		}
	}
*/
strcpy(URL,"");
strcpy(windowname,"wqwindow");
if(z == 1)
	strcpy(URL,"/cgi-bin/waitqueue.cgi");
if(z == 3)
	{
	strcpy(URL,"/cgi-bin/getmesg.cgi");
	sprintf(windowname,"win%d",lt);
	}
printf("Content-type: text/html\n\n");
//printf("<html><head><meta http-equiv=\"REFRESH\" CONTENT=\"300\"></head>\n");
printf("<html><head></head>\n");
printf("<script language=\"JavaScript\">\n");
printf("var TimerID2;\n");
printf("function reloadframe()\n");
printf("{\n");
printf("parent.nullspace.location.href = \"/cgi-bin/getstate.cgi?cam=%d?cookiename=%s\";\n",cam,cookiename);
printf("}\n");

printf("function onloadfunc() {\n");
if(z==1 || z==3)
	{
	printf("	%s=window.open('%s','%s','toolbar=no,location=no,diretories=no,status=no,menubar=no,scrollbars=yes,resizeable=no,width=320,height=150');\n",windowname,URL,windowname); 
	printf("	if(%s==null) || typeof(%s)==\"undefined\") alert(\"Someone tried to send you a reply, but since you failed to disable your popup blocker, you were unable to retrieve it.  Better luck next time.\");\n",windowname,windowname);
	}
//	printf("<body onload=\"%s=window.open('%s','%s','toolbar=no,location=no,diretories=no,status=no,menubar=no,scrollbars=yes,resizeable=no,width=320,height=150'); return false;\">\n",windowname,URL,windowname);
else if (z == -1)
	//printf("<body onload=\"parent.workspace.location.href='/theme0/primary.shtml?a=0&b=0&c=0&theme=0'\">\n");
	printf("	parent.workspace.location.href='/theme0/primary.shtml?a=0&b=0&c=0&theme=0';\n");

	printf("	TimerID2 = setTimeout(\"reloadframe()\",30000);\n");
	printf("}\n");
/*
else
	printf("<body>\n");
*/
printf("</script>\n");
printf("<body onload=\"onloadfunc()\">\n");
printf("<form NAME=\"form1\">\n");

for(x=0;x<inum;x++)
	{
	findtoken(ldt[x].name,buf,s);
	printf("<input TYPE=\"hidden\" NAME=\"%s\" VALUE=\"%d\">\n",ldt[x].name,atoi(s));
	}
printf("</form>\n");

printf("</body></html>\n");
}

oldprintstate(char	*buf)
{
int	x,y,z;
char	URL[256];
char	windowname[256];
long	lt;

time(&lt);
for(x=41;buf[x] != ' ';x++);
z = atoi(&buf[x+1]);
strcpy(URL,"");
strcpy(windowname,"wqwindow");
if(z == 1)
	strcpy(URL,"/cgi-bin/waitqueue.cgi");
if(z == 3)
	{
	strcpy(URL,"/cgi-bin/getmesg.cgi");
	sprintf(windowname,"win%d",lt);
	}
printf("Content-type: text/html\n\n");
printf("<html><head></head>\n");
if(z)
	printf("<body onload=\"%s=window.open('%s','%s','toolbar=no,location=no,diretories=no,status=no,menubar=no,scrollbars=yes,resizeable=no,width=320,height=150'); return false;\">\n",windowname,URL,windowname);
else
	printf("<body>\n");
printf("<form NAME=\"form1\">\n");
for(x=0;x<17;x++)
	{
	printf("<input TYPE=\"hidden\" NAME=\"lamp%d\" VALUE=\"%c\">\n",x,buf[x]);
	}
for(x=0;x<20;x++)
	{
	printf("<input TYPE=\"hidden\" NAME=\"comp%d\" VALUE=\"%c\">\n",x,buf[x+17]);
	}
printf("</body></html>\n");
}


int	getcamcount(int camnum)
{
int	sock;
int	rval;
int	num;
char	s[1024];
char	buf[2000];

sock = connect_socket("inferno",5201);
sprintf(s,"STATUS CAMCOUNT %d spaz",camnum);

if((write(sock, s, strlen(s)), 0) < 0)
	perror("writing on stream socket");

if(( rval = read(sock, buf, 1024)) < 0)
	perror("Reading from stream socket");
close(sock);
num = atoi(buf);
return(num);
}

int	listcamhosts(int camnum, char *retbuf)
{
int	sock;
int	rval;
int	num;
int	x,y,z;
char	s[1024];
char	buf[20000];

sock = connect_socket("inferno",5201);
sprintf(s,"STATUS LISTCAMHOSTS %d spaz",camnum);

if((write(sock, s, strlen(s)), 0) < 0)
	perror("writing on stream socket");

x = 0;
do
	{
	if(( rval = read(sock, &buf[x], 1460)) < 0)
		perror("Reading from stream socket");
	x = x + rval;
	}
while (rval >= 1460);
buf[x] = 0;
close(sock);
strcpy(retbuf,buf);
}

int	listcamcookies(int camcount,char *retbuf)
{
int	x,y,z;
int	size;
int	mycookienum;
int	handle;
int	owner;
char	buf[20000];
char	s[20000],t[2000],u[2000],temphost[2000];
char	mycookie[200];
char	cookiename[200];
struct	visitortype visitor;
struct	camtype	cam;

listcamhosts(camcount,buf);
y=0;
owner=0;
size=strlen(buf);
strcpy(retbuf,"");
if(getenv("REMOTE_ADDR") != NULL)
	strcpy(s,getenv("REMOTE_ADDR"));
else
	strcpy(s,"");
getcam(camcount,&cam);
mycookienum = getcookienumber();
getvisitorname(mycookienum, &visitor);
if(strcmp(visitor.nick,cam.account) == 0)
	owner=1;
getcookiename(s, NULL, mycookie);
sprintf(retbuf,"<!-- cn: %d  name: .%s.  account: .%s. -->\n",mycookienum,visitor.name,cam.account);
if(owner)
	strcat(retbuf,"<strong>This is your cam</strong><BR>\n");
for(z=0;z<size;z++)
	{
	if(buf[z] == '\n')
		{
		strncpy(s,&buf[y],z-y);
		s[z-y] = 0;
		strcpy(temphost,s);
		getcookiename(s, NULL, cookiename);
		strcpy(s,"");
		if(strcmp(mycookie,cookiename) == 0)
			strcat(s,"<FONT color=\"Green\">");
		strcat(s,cookiename);
		if(strcmp(mycookie,cookiename) == 0)
			strcat(s,"</FONT>");
		if(islistening(cookiename))
			strcat(s,"<FONT COLOR=\"Red\">*</FONT>");
		if(owner)
			{
			sprintf(t," (<A href=\"/cgi-bin/listcamhosts.cgi?cam=%d&blockname=%s\">Block</A>)",camcount,cookiename);
			strcat(s,t);
			}
		strcat(s,"<BR>\n");
		if(owner)
			{
			sprintf(t,"<FONT COLOR=\"Magenta\">(%s)</FONT><BR>\n",temphost);
			strcat(s,t);
			}
		if(cookiename[0] != 0)
			strcat(retbuf,s);
		y=z+1;
		}
	}
}

converttexttohtml(char *dest, char *src)
{
int	x,y,z;
char	*buf;

y = 0;
z=0;
while(src[z] != 0)
	{
	if(src[z] == '\n')
		{
		dest[y++] = '<';
		dest[y++] = 'B';
		dest[y++] = 'R';
		dest[y++] = '>';
		}
	else 
		dest[y++] = src[z];
	z++;
	}
dest[y] = 0;
}

/*
if(num == 0)
	printf("Nobody else is watching this cam.\n");
else if (num == 1)
	printf("There is 1 person watching this cam.\n");
else
	printf("There are %d people watching this cam.\n",num);
*/









getstate_function(char *retbuf,int camnum)
{
int	sock;
int	rval;
char	buf[2024];
char	s[1024];
int	x,y,z;
char	c;

char 	*fname;
int 	fd, status;
int	temp;
char	host[40];


if(getenv("REMOTE_ADDR") == NULL)
	strcpy(host,"spaz");
else
	strcpy(host,getenv("REMOTE_ADDR"));

temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */

	sock = connect_socket(LAMPSERVER_HOST, 5201);
	
	sprintf(s,"STATUS STATE %d %s",camnum,host);	
	
	
	if((write(sock, s, strlen(s)), 0) < 0)
		perror("writing on stream socket");
	
	if(( rval = read(sock, buf, 2000)) < 0)
		perror("Reading from stream socket");
	close(sock);
	strcpy(retbuf,buf);
	
}





int	cookieblocker()
{
int	x,y,z;
long	t;
struct	tm	*tim;
char	s[2000];
FILE	*blockfile;
FILE	*output;
char	filename[256];
char	searchstr[256];
char	host[2000];
int	mode;

// argument 1 - filename
// argument 2 - search string

mode = 0;
strcpy(filename,"/home/pjm/blocksc.dat");
strcpy(s,"");
x = cookiecheck(1,searchstr);
if(x == -1)
	return(0);

blockfile = fopen(filename,"r");
if(blockfile == NULL)
	exit(0);
fgets(s,256,blockfile);
s[256] = 0;
if(s[0] == 0)
	return(mode);
s[strlen(s)-1] = 0;
x = strlen(s);
while(!feof(blockfile))
	{
	if(x > 1)
		{
		if(strcmp(s,searchstr) == 0)
			{
			mode=1;
			if(getenv("REMOTE_ADDR") != NULL)
				strcpy(host,getenv("REMOTE_ADDR"));
			else
				strcpy(host,"INVALID");
			time(&t);
			tim = localtime(&t);
			output = fopen("/tmp/blocklog","a+");	
			fprintf(output,"Cookie block for %s from %s at %2.2d:%2.2d:%2.2d: on %d/%d/%d\n",searchstr,host,tim->tm_hour,tim->tm_min,tim->tm_sec,tim->tm_mon+1,tim->tm_mday,tim->tm_year);
			fflush(output);
			fclose(output);
			break;
			}
		}	
	fgets(s,256,blockfile);
	s[256] = 0;
	if(s[0] == 0)
		return(mode);
	s[strlen(s)-1] = 0;
	x = strlen(s);
	}
fclose(blockfile);
return(mode);
}

int	percamblocker(char *camname, char *cookiearg)
{
int	x,y,z;
long	t;
struct	tm	*tim;
char	s[2000];
FILE	*blockfile;
FILE	*output;
char	filename[256];
char	searchstr[256];
char	host[2000];
int	mode;

// argument 1 - filename
// argument 2 - search string

/*
if(cookiearg != NULL)
	fprintf(stderr,"camname: %s   cookiearg: .%s.\n",camname,cookiearg);
else
	fprintf(stderr,"camname: %s   cookiearg: .null.\n",camname);
*/
mode = 0;
sprintf(filename,"/extreme/blocks/%s",camname);
strcpy(s,"");

blockfile = fopen(filename,"r");
if(blockfile == NULL)
	return(0);
if(cookiearg == NULL)
	{
	x = cookiecheck(1,searchstr);
	fprintf(stderr,"cookiename: %s\n",searchstr);
	if(x == -1)
		return(0);
	}
else
	{
	if(cookiearg[0] == 0)
		return(0);
	strcpy(searchstr,cookiearg);
	}

fgets(s,256,blockfile);
s[256] = 0;
if(s[0] == 0)
	return(mode);
s[strlen(s)-1] = 0;
x = strlen(s);
while(!feof(blockfile))
	{
	if(x > 1)
		{
		if(strcmp(s,searchstr) == 0)
			{
			mode=1;
			if(getenv("REMOTE_ADDR") != NULL)
				strcpy(host,getenv("REMOTE_ADDR"));
			else
				strcpy(host,"INVALID");
			time(&t);
			tim = localtime(&t);
			output = fopen("/tmp/blocklog","a+");	
			fprintf(output,"Cam block [%s] for %s from %s at %2.2d:%2.2d:%2.2d: on %d/%d/%d\n",camname,searchstr,host,tim->tm_hour,tim->tm_min,tim->tm_sec,tim->tm_mon+1,tim->tm_mday,tim->tm_year);
			fflush(output);
			fclose(output);
			break;
			}
		}	
	fgets(s,256,blockfile);
	s[256] = 0;
	if(s[0] == 0)
		return(mode);
	s[strlen(s)-1] = 0;
	x = strlen(s);
	}
fclose(blockfile);
return(mode);
}

int	newpercamblocker(char *camname, char *accountname, char *cookiearg)
{
int	x,y,z;
long	t;
struct	tm	*tim;
char	s[2000];
FILE	*blockfile;
FILE	*output;
char	filename[256];
char	searchstr[256];
char	host[2000];
int	mode;

// argument 1 - filename
// argument 2 - search string

/*
if(cookiearg != NULL)
	fprintf(stderr,"camname: %s   cookiearg: .%s.\n",camname,cookiearg);
else
	fprintf(stderr,"camname: %s   cookiearg: .null.\n",camname);
*/
mode = 0;
sprintf(filename,"/extreme/blocks/%s",camname);
strcpy(s,"");

blockfile = fopen(filename,"r");
if(blockfile == NULL)
	return(0);
if(cookiearg == NULL)
	{
	x = cookiecheck(1,searchstr);
	fprintf(stderr,"cookiename: %s\n",searchstr);
	if(x == -1)
		return(0);
	}
else
	{
	if(cookiearg[0] == 0)
		return(0);
	strcpy(searchstr,cookiearg);
	}

fgets(s,256,blockfile);
s[256] = 0;
if(s[0] == 0)
	return(mode);
s[strlen(s)-1] = 0;
x = strlen(s);
while(!feof(blockfile))
	{
	if(x > 1)
		{
		if(strcmp(s,searchstr) == 0)
			{
			mode=1;
			if(getenv("REMOTE_ADDR") != NULL)
				strcpy(host,getenv("REMOTE_ADDR"));
			else
				strcpy(host,"INVALID");
			time(&t);
			tim = localtime(&t);
			output = fopen("/tmp/blocklog","a+");	
			fprintf(output,"Cam block [%s] for %s from %s at %2.2d:%2.2d:%2.2d: on %d/%d/%d\n",camname,searchstr,host,tim->tm_hour,tim->tm_min,tim->tm_sec,tim->tm_mon+1,tim->tm_mday,tim->tm_year);
			fflush(output);
			fclose(output);
			break;
			}
		}	
	fgets(s,256,blockfile);
	s[256] = 0;
	if(s[0] == 0)
		return(mode);
	s[strlen(s)-1] = 0;
	x = strlen(s);
	}
fclose(blockfile);
return(mode);
}




upcasestr(char *buf)
{
int     x,y,z;
x = strlen(buf);
for(z=0;z<x;z++)
	{
	if(buf[z] >= 'a' && buf[z] <= 'z')
		buf[z] = buf[z] - 'a' + 'A';
	}
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

int CIngrepit(char *buf, char *str, int	num)
{
int	x,y,z;
int	bufsize;
int	strsize;
char	s[10000];
char	t[1024];

//printf("searching for %s in %s.\n",str,buf);
if(num > 9998)
	num = 9998;
strncpy(s,buf,num);
strcpy(t,str);
//bufsize=strlen(s);
strsize=strlen(t);
upcasestr(s);
upcasestr(t);
for(z=0;z<num && s[z] != 0 ;z++)
	{
	if(strncmp(&s[z],t,strsize) == 0)
		{
//		printf("found %s it.\n",t);
		return(1);
		}
	}
return(0);
}

int	log_UA()
{
int	x,y,z;
char	s[2000];
FILE	*blockfile;
char	filename[256];
char	searchstr[256];
char	host[256];
char	addr[256];
char	name[256];
int	mode;

mode = 0;
strcpy(filename,"/home/pjm/UAwarn.dat");
if(getenv("HTTP_USER_AGENT") != NULL)
	strcpy(searchstr,getenv("HTTP_USER_AGENT"));
else
	return(0);
if(getenv("REMOTE_HOST") != NULL)
	strcpy(host,getenv("REMOTE_HOST"));
else
	return(0);
if(getenv("REMOTE_ADDR") != NULL)
	strcpy(addr,getenv("REMOTE_ADDR"));
else
	return(0);

blockfile = fopen(filename,"r");
if(blockfile == NULL)
	return(0);
fgets(s,256,blockfile);
s[256] = 0;
s[strlen(s)-1] = 0;
x = strlen(s);
while(!feof(blockfile))
	{
	fprintf(stderr,"UAwarn.  searchstr=.%s. s=.%s.\n",searchstr,s);
	if(CIgrepit(searchstr,s))
		{
		fprintf(stderr,"Got a match.\n");
		mode=1;
		break;
		}
	fgets(s,256,blockfile);
	s[256] = 0;
	s[strlen(s)-1] = 0;
	x = strlen(s);
	}
fclose(blockfile);
fprintf(stderr,"Done checking.\n");
if(mode)
	{
	getcookiename(addr,NULL,name);
	blockfile=fopen("/tmp/UAwarns.txt","a+");
	if(!ipblocker() && !cookieblocker())
		fprintf(blockfile,"Warning! UA match.  host: %s  UA: %s  User is not blocked. name: %s\n",host,searchstr,name);
	else
		fprintf(blockfile,"Warning! UA match.  host: %s  UA: %s  User is blocked. name: %s\n",host,searchstr,name);
	fclose(blockfile);
	}
return(mode);
}

int	refblocker()
{
int	x,y,z;
char	s[2000];
FILE	*blockfile;
char	filename[256];
char	searchstr[256];
int	mode;

mode = 0;
strcpy(filename,"/home/pjm/blocksref.dat");
if(getenv("HTTP_REFERER") != NULL)
	strcpy(searchstr,getenv("HTTP_REFERER"));
else
	return(0);

blockfile = fopen(filename,"r");
if(blockfile == NULL)
	return(0);
fgets(s,256,blockfile);
s[256] = 0;
s[strlen(s)-1] = 0;
x = strlen(s);
while(!feof(blockfile))
	{
//	fprintf(stderr,"refblocker.  searchstr=.%s. s=.%s.\n",searchstr,s);
	if(CIgrepit(searchstr,s))
		{
		fprintf(stderr,"Got a match.\n");
		mode=1;
		break;
		}
	fgets(s,256,blockfile);
	s[256] = 0;
	s[strlen(s)-1] = 0;
	x = strlen(s);
	}
fclose(blockfile);
return(mode);
}


int	ipblocker()
{
int	x,y,z;
long	t;
struct	tm	*tim;
char	s[2000];
FILE	*blockfile;
FILE	*output;
char	filename[256];
char	searchstr[256];
char	host[2000];
int	mode;

// argument 1 - filename
// argument 2 - search string

mode = 0;
strcpy(filename,"/home/pjm/blocksi.dat");
if(getenv("REMOTE_ADDR") != NULL)
	strcpy(searchstr,getenv("REMOTE_ADDR"));
else
	return(0);

blockfile = fopen(filename,"r");
if(blockfile == NULL)
	return(0);
fgets(s,256,blockfile);
s[256] = 0;
s[strlen(s)-1] = 0;
x = strlen(s);
while(!feof(blockfile))
	{
	if(x > 3)
		{
		if(strncmp(s,searchstr,x) == 0)
			{
			mode=1;
			if(getenv("REMOTE_ADDR") != NULL)
				strcpy(host,getenv("REMOTE_ADDR"));
			else
				strcpy(host,"INVALID");
			time(&t);
			tim = localtime(&t);
			output = fopen("/tmp/blocklog","a+");	
			fprintf(output,"Cookie block for %s from %s at %2.2d:%2.2d:%2.2d: on %d/%d/%d\n",searchstr,host,tim->tm_hour,tim->tm_min,tim->tm_sec,tim->tm_mon+1,tim->tm_mday,tim->tm_year);
			fflush(output);
			fclose(output);
			break;
			}
		}
	fgets(s,256,blockfile);
	s[256] = 0;
	s[strlen(s)-1] = 0;
	x = strlen(s);
	}
fclose(blockfile);
return(mode);
}




int	commentlink(char *qs)
{
int	sock;
int	rval;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();
char	buf[10010];
char	s[1024];
int	x,y,z;
char	c;
FILE	*in;

char 	*fname;
int 	fd, status;
int	temp;





temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */



	sock = connect_socket(COMMENTSERVER_HOST, 5204);
	sprintf(s,"NUMBER %s",qs);
	
	
	if((write(sock, s, strlen(s)), 0) < 0)
		perror("writing on stream socket");
	
	if(( rval = read(sock, buf, 10000)) < 0)
		perror("Reading from stream socket");
		
	close(sock);
	printf("<A HREF=\"/cgi-bin/newcomments.cgi?name=%s\">Comments(%d)</A>\n",qs,atoi(buf));
}



		

print_cam_archives(int	br, int cam)
{
int	x,y,z;
int	refresh_rate;
int	browser;
int	type;
char	host[256];
int	theme;
int	port;
int	handle;
int	size;
int	numcams;
struct	camtype *cams;
char 	filename[256];


size = getnumcams();
numcams = size;
cams = malloc(size * sizeof cams[0]);
for(z=0;z<size;z++)
        {
	getcam(z,&cams[z]);
/*
        lseek(handle,z*sizeof cams[0],0);
        read(handle,&cams[z], sizeof cams[0]);
*/
        }
//close(handle);


theme=gettheme();
type=cams[cam].type;
refresh_rate = cams[cam].refresh_rate;
if(br < 10)
	{
	if(cams[cam].cd[br].available == 0)
		browser=0;
	else
		browser=br;
	}
else
	browser=br;
if(browser < 10)
	{
	strcpy(host,cams[cam].cd[browser].host);
	port = cams[cam].cd[browser].port;
	strcpy(filename,cams[cam].cd[browser].filename);
	}
if((browser == 0 || type == 1))
	{
	if(refresh_rate > 0)
		{
		printf("<SCRIPT LANGUAGE=\"JavaScript\">\n");
		printf("<!--\n");
		printf("bName = navigator.appName;\n");
		printf("var isopen%d=Math.round(1000000+(Math.random() * 1000000));\n",cam);
		printf("var Timeoutcam%d;\n",cam);
		printf("var camurl%d=\"http://%s:%d%s?\";\n",cam,host,port,filename);
		printf("tmpimage%d = new Image();var first%d=1; var doneloading%d=0;\n",cam,cam,cam);
		printf("function seturl%d(txt)\n",cam);
		printf("{ camurl%d = txt; }\n",cam);
	
		printf("function doimage%d()\n",cam);
		printf("{\n");
		printf("if(doneloading%d) {\n",cam);
		printf("doneloading%d=0;\n",cam);
		printf("isopen%d++;\n",cam);
		printf("document.images.webcam%d.src=tmpimage%d.src;\n",cam,cam);
		printf("tmpimage%d.src=camurl%d+isopen%d;\n",cam,cam,cam,host,port,filename);
		printf("Timeoutcam%d = setTimeout('doimage%d()',%d);\n",cam,cam,refresh_rate*1000);
		printf("} else { Timeoutcam%d = setTimeout('doimage%d()',1000); }\n",cam,cam);
		printf("}\n");

		printf("\n");
		printf("function imageonload%d()\n",cam);
		printf("{\n");
		printf("doneloading%d=1;\n",cam);
		printf("}\n");
	
		printf("function firstimage%d()\n",cam);
		printf("	{\n");
		printf("	if (first%d == 1) {\n",cam);
		printf("		first%d = 0;\n",cam);
		printf("		isopen%d++;\n",cam);
		printf("		tmpimage%d.onload=imageonload%d;\n",cam,cam);
		printf("		tmpimage%d.src=camurl%d+isopen%d;\n",cam,cam,cam,host,port,filename);
		printf("		Timeoutcam%d=setTimeout('doimage%d()',%d);\n",cam,cam,refresh_rate*1000);
		printf("		}\n	}\n");
		printf("document.write('<A HREF=\"/cgi-bin/point1.cgi\" target=nullspace><IMG border=0 src=\"http://%s:%d%s\" name=webcam%d onload=\"firstimage%d()\" height=\"240\" width=\"320\" ismap></A>');\n",host,port,filename,cam,cam);
		printf("//-->\n");
		printf("</SCRIPT>\n");
		printf("\n");
		}
	else
		{
		printf("<SCRIPT LANGUAGE=\"JavaScript\">\n");
		printf("<!--\n");
		printf("bName = navigator.appName;\n");
		printf("var isopen%d=Math.round(1000000+(Math.random() * 1000000));\n",cam);
		printf("var camurl%d=\"http://%s:%d%s?\";\n",cam,host,port,filename);
		printf("tmpimage%d = new Image();var first%d=1;\n",cam,cam);
		printf("function seturl%d(txt)\n",cam);
		printf("{ camurl%d = txt; }\n",cam);
	
		printf("function doimage%d()\n",cam);
		printf("{\n");
		printf("isopen%d++;\n",cam);
		printf("document.images.webcam%d.src=tmpimage%d.src;\n",cam,cam);
		printf("tmpimage%d.src=camurl%d+isopen%d;\n",cam,cam,cam,host,port,filename);
		printf("}\n");
	
		printf("function firstimage%d()\n",cam);
		printf("{\n");
		printf("if (first%d == 1) {\n",cam);
		printf("first%d = 0;\n",cam);
		printf("isopen%d++;\n",cam);
		printf("tmpimage%d.onload=doimage%d;\n",cam,cam);
		printf("tmpimage%d.src=camurl%d+isopen%d;\n",cam,cam,cam,host,port,filename);
		printf("} }\n");
		printf("document.write('<A HREF=\"/cgi-bin/point1.cgi\" target=nullspace><IMG border=0 src=\"http://%s:%d%s\" name=webcam%d onload=\"firstimage%d()\" height=\"240\" width=\"320\" ismap></A>');\n",host,port,filename,cam,cam);
		printf("//-->\n");
		printf("</SCRIPT>\n");
		printf("\n");
		}
	}
else if(browser == 1)
	{
	printf("<A HREF=\"/cgi-bin/point1.cgi\" target=nullspace><IMG border=0 src=\"http://%s:%d%s\" height=\"240\" width=\"320\" ismap></A>\n",host,port,filename);
	}
else if(browser == 2)
	{


	printf("<OBJECT id=MediaPlayer1 classid=CLSID:6BF52A52-394A-11d3-B153-00C04F79FAA6\n"); 
	printf("align=middle width=320 height=240 type=application/x-oleobject>\n");
	printf("<PARAM NAME=\"URL\" VALUE=\"http://%s:%d%s\">\n",host,port,filename);
	printf("<PARAM NAME=\"rate\" VALUE=\"1\"><PARAM NAME=\"balance\"      `VALUE=\"0\">\n");
	printf("<PARAM NAME=\"currentPosition\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"defaultFrame\" VALUE=\"mainFrame\">\n");
	printf("<PARAM NAME=\"playCount\" VALUE=\"1\">\n");
	printf("<PARAM NAME=\"autoStart\" VALUE=\"-1\">\n");
	printf("<PARAM NAME=\"currentMarker\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"invokeURLs\" VALUE=\"-1\">\n");
	printf("<PARAM NAME=\"baseURL\" VALUE=\"\">\n");
	printf("<PARAM NAME=\"volume\" VALUE=\"50\">\n");
	printf("<PARAM NAME=\"mute\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"uiMode\" VALUE=\"none\">\n");
	printf("<PARAM NAME=\"stretchToFit\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"windowlessVideo\" VALUE=\"0\">\n");
	printf("    <PARAM NAME=\"ShowControls\" VALUE=\"0\">\n");
	printf("    <PARAM NAME=\"ShowStatusBar\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"enabled\" VALUE=\"-1\">\n");
	printf("<PARAM NAME=\"BufferingTime\" VALUE=\"1\">\n");
	printf("<PARAM NAME=\"enableContextMenu\" VALUE=\"-1\">\n");
	printf("<PARAM NAME=\"fullScreen\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"SAMIStyle\" VALUE=\"\">\n");
	printf("<PARAM NAME=\"SAMILang\" VALUE=\"\">\n");
	printf("<PARAM NAME=\"SAMIFilename\" VALUE=\"\">\n");
	printf("<PARAM NAME=\"captioningID\" VALUE=\"\">\n");

	printf("<embed type=\"application/x-mplayer2\"\n"); 
	printf("pluginspage = \"http://www.microsoft.com/Windows/MediaPlayer/\"\n");
	printf("src=\"http://%s:%d%s\" align=\"middle\"\n",host,port,filename);
	printf("width=\"320\" height=\"286\" defaultframe=\"rightFrame\" ShowStatusBar=false ShowControls=false> </embed>   </OBJECT>\n");

	}
else if(browser == 3)
	{
	printf("<applet code=\"kutttpech.KutttPech\"  codebase=\"http://206.54.175.158/\" archive=\"kutttpech-0.4.1.jar\" width=\"320\" height=\"240\">\n");
	printf("<param name=\"FILENAME\" value=\"http://%s:%d%s\">\n",host,port,filename);
	printf("</applet>\n");
	}
else if(browser == 10) // cache
	{
	char temps[2000];
	refresh_rate = cams[cam].update_rate;
	if(cams[cam].archive && refresh_rate < cams[cam].archive_rate)
		refresh_rate = cams[cam].archive_rate;
	http_encode(temps,cams[cam].name);
	sprintf(filename,"/cgi-bin/dispcache.cgi?name=%s",temps);
	printf("<SCRIPT LANGUAGE=\"JavaScript\">\n");
	printf("<!--\n");
	printf("bName = navigator.appName;\n");
	printf("var isopen=Math.round(1000000+(Math.random() * 1000000));\n");
	printf("var Timeoutcam%d;\n",cam);
	printf("var camurl%d=\"%s&\";\n",cam,filename);
	printf("tmpimage%d = new Image();var first=1; var doneloading%d=0;\n",cam,cam);
	printf("function seturl%d(txt)\n",cam);
	printf("{ camurl%d = txt; }\n",cam);

	printf("function doimage%d()\n",cam);
	printf("{\n");
	printf("if(doneloading%d) {\n",cam);
	printf("doneloading%d=0;\n",cam);
	printf("isopen++;\n");
	printf("document.images.webcam%d.src=tmpimage%d.src;\n",cam,cam);
	printf("tmpimage%d.src=camurl%d+isopen;\n",cam,cam,host,port,filename);
	printf("Timeoutcam%d = setTimeout('doimage%d()',%d);\n",cam,cam,refresh_rate*1000);
	printf("} else { Timeoutcam%d = setTimeout('doimage%d()',1000); }\n",cam,cam);
	printf("}\n");

	printf("\n");
	printf("function imageonload%d()\n",cam);
	printf("{\n");
	printf("doneloading%d=1;\n",cam);
	printf("}\n");

	printf("function firstimage%d()\n",cam);
	printf("	{\n");
	printf("	if (first == 1) {\n");
	printf("		first = 0;\n");
	printf("		isopen++;\n");
	printf("		tmpimage%d.onload=imageonload%d;\n",cam,cam);
	printf("		tmpimage%d.src=camurl%d+isopen;\n",cam,cam,host,port,filename);
	printf("		Timeoutcam%d=setTimeout('doimage%d()',%d);\n",cam,cam,refresh_rate*1000);
	printf("		}\n	}\n");
	printf("document.write('<A HREF=\"/cgi-bin/point1.cgi\" target=nullspace><IMG border=0 src=\"%s\" name=webcam%d onload=\"firstimage%d()\" height=\"240\" width=\"320\" ismap></A>');\n",filename,cam,cam);
	printf("//-->\n");
	printf("</SCRIPT>\n");
	printf("\n");
	}
else if(browser == 11) // archive
	{
	char temps[2000];
	int	handle;
	int	filesize;

	refresh_rate = 1;
	http_encode(temps,cams[cam].name);
	sprintf(filename,"/extreme/archives/data/%s",cams[cam].name);
	handle = open(filename,O_RDONLY);
	if(handle == -1)
		return(0);
	filesize = lseek(handle,0,2);
	close(handle);
	if(filesize <= 0)
		return(0);
	printf("<strong><center>%s</center></strong>\n",cams[cam].name);
	sprintf(filename,"/cgi-bin/disparchive.cgi?name=%s&record=",temps);
	printf("<SCRIPT LANGUAGE=\"JavaScript\">\n");
	printf("<!--\n");
	printf("bName = navigator.appName;\n");
	printf("var isopen=Math.round(1000000+(Math.random() * 1000000));\n");
	printf("var Timeoutcam%d;\n",cam);
	printf("var camurl%d=\"%s\";\n",cam,filename);
	printf("tmpimage%d = new Image();var first=1; var doneloading%d=0;\n",cam,cam);
	printf("function seturl%d(txt)\n",cam);
	printf("{ camurl%d = txt; }\n",cam);

	printf("function doimage%d()\n",cam);
	printf("{\n");
	printf("var now = new Date();\n");
	printf("if(doneloading%d) {\n",cam);
	printf("doneloading%d=0;\n",cam);
	printf("isopen++;\n");
	printf("document.images.webcam%d.src=tmpimage%d.src;\n",cam,cam);
	printf("tmpimage%d.src=camurl%d+now.getTime()/1000;\n",cam,cam,host,port,filename);
	printf("Timeoutcam%d = setTimeout('doimage%d()',%d);\n",cam,cam,refresh_rate*1000);
	printf("} else { Timeoutcam%d = setTimeout('doimage%d()',1000); }\n",cam,cam);
	printf("}\n");

	printf("\n");
	printf("function imageonload%d()\n",cam);
	printf("{\n");
	printf("doneloading%d=1;\n",cam);
	printf("}\n");

	printf("function firstimage%d()\n",cam);
	printf("	{\n");
	printf("	var now = new Date();\n");
	printf("	if (first == 1) {\n");
	printf("		first = 0;\n");
	printf("		isopen++;\n");
	printf("		tmpimage%d.onload=imageonload%d;\n",cam,cam);
	printf("		tmpimage%d.src=camurl%d+now.getTime()/1000;\n",cam,cam,host,port,filename);
	printf("		Timeoutcam%d=setTimeout('doimage%d()',%d);\n",cam,cam,refresh_rate*1000);
	printf("		}\n	}\n");
	printf("document.write('<A HREF=\"/cgi-bin/point1.cgi\" target=nullspace><IMG border=0 src=\"%s0\" name=webcam%d onload=\"firstimage%d()\" height=\"240\" width=\"320\" ismap></A>');\n",filename,cam,cam);
	printf("//-->\n");
	printf("</SCRIPT>\n");
	printf("\n");
	
	}
else if(browser == 6)
	{


	printf("<OBJECT id=MediaPlayer1 classid=CLSID:6BF52A52-394A-11d3-B153-00C04F79FAA6\n"); 
	printf("align=middle width=320 height=240 type=application/x-oleobject>\n");
	printf("<PARAM NAME=\"URL\" VALUE=\"http://%s:%d%s\">\n",host,port,filename);
	printf("<PARAM NAME=\"rate\" VALUE=\"1\"><PARAM NAME=\"balance\"      `VALUE=\"0\">\n");
	printf("<PARAM NAME=\"currentPosition\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"defaultFrame\" VALUE=\"mainFrame\">\n");
	printf("<PARAM NAME=\"playCount\" VALUE=\"1\">\n");
	printf("<PARAM NAME=\"autoStart\" VALUE=\"-1\">\n");
	printf("<PARAM NAME=\"currentMarker\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"invokeURLs\" VALUE=\"-1\">\n");
	printf("<PARAM NAME=\"baseURL\" VALUE=\"\">\n");
	printf("<PARAM NAME=\"volume\" VALUE=\"50\">\n");
	printf("<PARAM NAME=\"mute\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"uiMode\" VALUE=\"none\">\n");
	printf("<PARAM NAME=\"stretchToFit\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"windowlessVideo\" VALUE=\"0\">\n");
	printf("    <PARAM NAME=\"ShowControls\" VALUE=\"0\">\n");
	printf("    <PARAM NAME=\"ShowStatusBar\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"enabled\" VALUE=\"-1\">\n");
	printf("<PARAM NAME=\"BufferingTime\" VALUE=\"1\">\n");
	printf("<PARAM NAME=\"enableContextMenu\" VALUE=\"-1\">\n");
	printf("<PARAM NAME=\"fullScreen\" VALUE=\"0\">\n");
	printf("<PARAM NAME=\"SAMIStyle\" VALUE=\"\">\n");
	printf("<PARAM NAME=\"SAMILang\" VALUE=\"\">\n");
	printf("<PARAM NAME=\"SAMIFilename\" VALUE=\"\">\n");
	printf("<PARAM NAME=\"captioningID\" VALUE=\"\">\n");

	printf("<embed type=\"application/x-mplayer2\"\n"); 
	printf("pluginspage = \"http://www.microsoft.com/Windows/MediaPlayer/\"\n");
	printf("src=\"http://%s:%d%s\" align=\"middle\"\n",host,port,filename);
	printf("width=\"320\" height=\"286\" defaultframe=\"rightFrame\" ShowStatusBar=false ShowControls=false> </embed>   </OBJECT>\n");

	}
else if((browser >= 7 && browser <= 9))
	{
	refresh_rate = 5;
	if(refresh_rate > 0)
		{
		printf("<SCRIPT LANGUAGE=\"JavaScript\">\n");
		printf("<!--\n");
		printf("bName = navigator.appName;\n");
		printf("var isopen=Math.round(1000000+(Math.random() * 1000000));\n");
		printf("var Timeoutcam%d;\n",cam);
		printf("var camurl%d=\"http://%s:%d%s?\";\n",cam,host,port,filename);
		printf("tmpimage%d = new Image();var first=1; var doneloading%d=0;\n",cam,cam);
		printf("function seturl%d(txt)\n",cam);
		printf("{ camurl%d = txt; }\n",cam);
	
		printf("function doimage%d()\n",cam);
		printf("{\n");
		printf("if(doneloading%d) {\n",cam);
		printf("doneloading%d=0;\n",cam);
		printf("isopen++;\n");
		printf("document.images.webcam%d.src=tmpimage%d.src;\n",cam,cam);
		printf("tmpimage%d.src=camurl%d+isopen;\n",cam,cam,host,port,filename);
		printf("Timeoutcam%d = setTimeout('doimage%d()',%d);\n",cam,cam,refresh_rate*1000);
		printf("} else { Timeoutcam%d = setTimeout('doimage%d()',1000); }\n",cam,cam);
		printf("}\n");

		printf("\n");
		printf("function imageonload%d()\n",cam);
		printf("{\n");
		printf("doneloading%d=1;\n",cam);
		printf("}\n");
	
		printf("function firstimage%d()\n",cam);
		printf("	{\n");
		printf("	if (first == 1) {\n");
		printf("		first = 0;\n");
		printf("		isopen++;\n");
		printf("		tmpimage%d.onload=imageonload%d;\n",cam,cam);
		printf("		tmpimage%d.src=camurl%d+isopen;\n",cam,cam,host,port,filename);
		printf("		Timeoutcam%d=setTimeout('doimage%d()',%d);\n",cam,cam,refresh_rate*1000);
		printf("		}\n	}\n");
		printf("document.write('<A HREF=\"/cgi-bin/point1.cgi\" target=nullspace><IMG border=0 src=\"http://%s:%d%s\" name=webcam%d onload=\"firstimage%d()\" height=\"240\" width=\"320\" ismap></A>');\n",host,port,filename,cam,cam);
		printf("//-->\n");
		printf("</SCRIPT>\n");
		printf("\n");
		}
	else
		{
		printf("<SCRIPT LANGUAGE=\"JavaScript\">\n");
		printf("<!--\n");
		printf("bName = navigator.appName;\n");
		printf("var isopen=Math.round(1000000+(Math.random() * 1000000));\n");
		printf("var camurl%d=\"http://%s:%d%s?\";\n",cam,host,port,filename);
		printf("tmpimage%d = new Image();var first=1;\n",cam);
		printf("function seturl%d(txt)\n",cam);
		printf("{ camurl%d = txt; }\n",cam);
	
		printf("function doimage%d()\n",cam);
		printf("{\n");
		printf("isopen++;\n");
		printf("document.images.webcam%d.src=tmpimage%d.src;\n",cam,cam);
		printf("tmpimage%d.src=camurl%d+isopen;\n",cam,cam,host,port,filename);
		printf("}\n");
	
		printf("function firstimage%d()\n",cam);
		printf("{\n");
		printf("if (first == 1) {\n");
		printf("first = 0;\n");
		printf("isopen++;\n");
		printf("tmpimage%d.onload=doimage%d;\n",cam,cam);
		printf("tmpimage%d.src=camurl%d+isopen;\n",cam,cam,host,port,filename);
		printf("} }\n");
		printf("document.write('<A HREF=\"/cgi-bin/point1.cgi\" target=nullspace><IMG border=0 src=\"http://%s:%d%s\" name=webcam%d onload=\"firstimage%d()\" height=\"240\" width=\"320\" ismap></A>');\n",host,port,filename,cam,cam);
		printf("//-->\n");
		printf("</SCRIPT>\n");
		printf("\n");
		}
	}
else
	{
	printf("<IMG border=0 src=\"/pics/house320x240.jpg\" height=\"240\" width=\"320\" usemap=\"#map1\">\n",host,port,filename);
	printf("<MAP name=\"map1\">\n");
	printf("<AREA nohref shape=\"rect\" coords=\"230,62,304,114\" onMouseOver=\"seturl10('http://206.54.177.102/singleframe.jpg')\">\n");
	printf("<AREA nohref shape=\"rect\" coords=\"231,145,303,230\" onMouseOver=\"seturl10('http://206.54.177.104:80/singleframe.jpg')\">\n");
	printf("<AREA nohref shape=\"rect\" coords=\"234,116,319,143\" onMouseOver=\"seturl10('http://206.54.177.100:80/singleframe.jpg')\">\n");
	printf("<AREA nohref shape=\"rect\" coords=\"306,0,319,239\" onMouseOver=\"seturl10('http://206.54.177.100:80/singleframe.jpg')\">\n");
	printf("<AREA nohref shape=\"rect\" coords=\"109,143,182,228\" onMouseOver=\"seturl10('http://206.54.177.103:80/singleframe.jpg')\">\n");
	printf("<AREA nohref shape=\"rect\" coords=\"171,7,217,66\" onMouseOver=\"seturl10('http://206.54.177.110:81/singleframe.jpg')\">\n");
	printf("</MAP>\n");
	}
}






int	getcambyname(char *camname)
{
int	x,y,z;
int	size;
int	handle;
struct	camtype cam;

size = getnumcams();
for(z=0;z<size;z++)
	{
	getcam(z,&cam);
	if(strcmp(cam.name,camname) == 0)
		{
		return(z);
		}
	}
return(-1);
}


int	createcommentname(char *directory, char *filename, char *retbuf)
{
int	x,y,z;
char	s[2000];
char	tmp[2000];

strcpy(tmp,directory);
for(z=0;z<strlen(tmp);z++)
	if(tmp[z] == '/')
		tmp[z] = '0';
sprintf(s,"%s0%s",tmp,filename);
strcpy(retbuf,s);
}

int	resizedimage(char *directory, char *filename, char *dimensions )
{
int	x,y,z;
char	s[2000],t[2000];
char	cn[2000];
char	pn[2000];
char	fn[2000];
int	size;
int	handle;
int	pid;
char	*buffer;

pid = getpid();
sprintf(t,"%s0%s",filename,dimensions);
createcommentname(directory,t,cn);
sprintf(pn,"/extreme/imagecache/%s",cn);
sprintf(fn,"/var/lib/httpd/htdocs%s/%s",directory,filename);
fprintf(stderr,"t: .%s. pn: .%s.  fn: .%s.\n",t,pn,fn);
handle = open(pn,O_RDONLY);
if(handle != -1)
	{
	// found a cached image.
	size = lseek(handle,0,2);
	lseek(handle,0,0);
	buffer = malloc(size+5);
	read(handle,buffer,size);
	close(handle);
	printf("Content-type: image/jpg\n\n");
	for(z=0;z<size;z++)
		putc(buffer[z],stdout);
	free(buffer);
	}
else
	{
	fprintf(stderr,"handle = -1\n");
	handle = open(fn,O_RDONLY);
	if(handle == -1)
		exit(1);
	close(handle);
//mogrify stuff	
	sprintf(s,"cp %s /tmp/cachetmp%d",fn,pid);
	system(s);
	sprintf(s,"/usr/X11R6/bin/mogrify -antialias -geometry %s /tmp/cachetmp%d",dimensions,pid); 
	system(s);
	sprintf(s,"mv /tmp/cachetmp%d %s",pid,pn);
	system(s);
	handle = open(pn,O_RDONLY);
	if(handle == -1)
		{
		fprintf(stderr,"Freak out.\n");
		exit(1);
		}
	size = lseek(handle,0,2);
	fprintf(stderr,"size = %d\n",size);
	lseek(handle,0,0);
	buffer = malloc(size+5);
	read(handle,buffer,size);
	close(handle);
	printf("Content-type: image/jpg\n\n");
	for(z=0;z<size;z++)
		putc(buffer[z],stdout);
	free(buffer);
	}

}

int	print_comment(char *commentname, char *name, int theme)
{
int	sock;
int	rval;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();
char	buf[50010];
char	s[20024];
int	x,y,z;
char	c;
FILE	*in;

char 	*fname;
int 	fd, status;
int	temp;
char	qs[2000];
char	*retbuf;
char	*itembuf;
char	*tempbuf;
struct	visitortype visitor;
struct	visitortype myvisitor;


y = getcookienumber();
getvisitorname(y, &myvisitor);
temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */


sock = connect_socket("doorbell",5204);
fprintf(stderr,"Connected.\n");
sprintf(s,"LIST %s",commentname);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writing on stream socket");
fprintf(stderr,"Sent command.\n");
x = 0;	
do
	{
	if(( rval = read(sock, &buf[x], 1460)) < 0)
		perror("Reading from stream socket");
	x = x + rval;
	fprintf(stderr,"Reading.  x=%d  rval=%d\n",x,rval);
	}
while(rval > 0);
//	fprintf(stderr,"Done reading. buf= .%s.\n",buf);	
close(sock);
x=0;
y=0;
myerror("done reading.");

printf("<table border=1>\n");
if(buf[0] == 0)
	{
	printf("<TR><TD>No Comments</TD></TR>\n");
	}
else	
	{
	retbuf = xmlparse("comments",0,buf);
	x = xmlcount("item",retbuf);
	if(x <= 0)
		{
		printf("<TR><TD>No Comments</TD></TR>\n");
		}
	for(z=0;z<x;z++)
		{
		sprintf(s,"Now doing %d.  Total %d",z,x);
		myerror(s);
		printf("<tr><td>\n");
		itembuf = xmlparse("item",z,buf);
		tempbuf = xmlparse("account",0,itembuf);
		myerror("Got buffers.");
		if(tempbuf != NULL && tempbuf[0] != 0)
			{
			fprintf(stderr,"account: .%s.\n",tempbuf);
			y = getvisitorbyname(tempbuf,&visitor);
			sprintf(s,"Got visitor: .%s. y=%d",visitor.nick,y);
			myerror(s);
			fprintf(stderr,"visitor: .%s.\n",visitor.nick);
			myerror("Checking y.\n");
			if(y > -1)
				{
				myerror("Got y.  chekcing minipic\n");
				if(visitor.minipicfilename[0] != 0)
					{
					printf("<IMG align=left width=100 height=100 src=\"/submitpics/%s\">\n",visitor.minipicfilename);
					}
				}
			myerror("About to free account");
			free(tempbuf);
			myerror("Done with account.");
			}
		tempbuf = xmlparse("nick",0,itembuf);
		printf("Nick: <STRONG>%s</STRONG><BR>\n",tempbuf);
		free(tempbuf);
		
		tempbuf = xmlparse("date",0,itembuf);
		printf("Date: <STRONG>%s</STRONG><P>\n",tempbuf);
		free(tempbuf);
		
		tempbuf = xmlparse("text",0,itembuf);
		printf("%s\n",tempbuf);
		free(tempbuf);
		
		tempbuf = xmlparse("account",0,itembuf);
		if(tempbuf != NULL && tempbuf[0] != 0)
			{
			myerror("doing admin stuff.");
			if(strcmp(tempbuf,myvisitor.nick) == 0 || adminname(myvisitor.nick))
				{
				printf("<BR><a href=\"/cgi-bin/deletecomment.cgi?name=%s&record=%d&username=%s&password=%s&account=%s\">Delete</a>\n",commentname,z,myvisitor.nick,myvisitor.password,tempbuf);
				printf("<BR><a href=\"/cgi-bin/censorcomment.cgi?name=%s&record=%d&username=%s&password=%s&account=%s\">Censor</a>\n",commentname,z,myvisitor.nick,myvisitor.password,tempbuf);
				}
			free(tempbuf);
			}
		else if(adminname(myvisitor.nick))
			{
			printf("<BR><a href=\"/cgi-bin/deletecomment.cgi?name=%s&record=%d\">Delete</a>\n",commentname,z);
			printf("<BR><a href=\"/cgi-bin/censorcomment.cgi?name=%s&record=%d&username=%s&password=%s&account=%s\">Censor</a>\n",commentname,z,myvisitor.nick,myvisitor.password,tempbuf);
			}
		free(itembuf);
		printf("</td></tr>\n");
		}
	free(retbuf);
	}
/*
	for(x=y;buf[x] != '\n' && buf[x] != 0;x++);
	strncpy(s,&buf[y],x-y);
	s[x-y] = 0;
	y=x+1;
	if(strcmp(s,"<.>") == 0)
		{
		printf("<tr><td>\n");
		y++;
		for(x=y;buf[x] != '>';x++);
		strncpy(s,&buf[y],x-y);
		s[(x-y)] = 0;
		printf("Nick: <STRONG>%s</STRONG><BR>\n",s);
		y=x+3;
		
		for(x=y;buf[x] != '>';x++);
		strncpy(s,&buf[y],x-y);
		s[(x-y)] = 0;
		printf("Date: <STRONG>%s</STRONG><P>\n",s);
		y=x+2;
			
//			fprintf(stderr,"got here.\n");
		for(x=y;buf[x] != '>';x++);
		y=x+2;
		for(x=y;strncmp(&buf[x],"<end>",5) != 0 && buf[x] != 0;x++);
		strncpy(s,&buf[y],x-y);
		s[(x-y)] = 0;
		printf("%s\n",s);
		y=x;

//			fprintf(stderr,"got here too.\n");	

		for(x=y;buf[x] != '>';x++);
		y=x+2;
		
		printf("</td></tr>\n");	
		}
	else
		continue;
			
	}
//	fprintf(stderr,"%s\n",buf);
*/
myerror("done with comments.");
printf("</TABLE><P><P><P>\n");
printf("Enter your comment here:<P>\n");
printf("<FORM action=\"/cgi-bin/newaddcomment.cgi\" method=\"POST\">\n");
printf("<input type=\"hidden\" NAME=\"h1\" VALUE=\"%s\">\n",commentname);
printf("Nick: <input type=\"text\" NAME=\"n1\" VALUE=\"%s\">\n",name);
printf("<P>Comment:<BR>\n");
printf("<TEXTAREA NAME=\"q1\" ROWS=6 COLS=60></TEXTAREA><P>\n");
printf("<input type=\"submit\" VALUE=\"SUBMIT\">\n");
printf("</FORM><P><P><P>\n");
printf("There are no specific posting guidelines... (yet)..  however, excessively obscene, abusive, harrassing, trollish, or posts with illegal content may be removed at the discretion of the site owner.\n"); 
//printf("</TD></TR></TABLE>\n");
}

int	oldprint_comment(char *commentname, char *name, int theme)
{
int	sock;
int	rval;
struct	sockaddr_in server;
struct	hostent *hp, *gethostbyname();
char	buf[50010];
char	s[20024];
int	x,y,z;
char	c;
FILE	*in;

char 	*fname;
int 	fd, status;
int	temp;
char	qs[2000];


temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */


sock = connect_socket("doorbell",5203);
fprintf(stderr,"Connected.\n");
sprintf(s,"LIST %s",commentname);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writing on stream socket");
fprintf(stderr,"Sent command.\n");
x = 0;	
do
	{
	if(( rval = read(sock, &buf[x], 1460)) < 0)
		perror("Reading from stream socket");
	x = x + rval;
	fprintf(stderr,"Reading.  x=%d  rval=%d\n",x,rval);
	}
while(rval > 0);
//	fprintf(stderr,"Done reading. buf= .%s.\n",buf);	
close(sock);
x=0;
y=0;
myerror("done reading.");

printf("<table border=1>\n");
if(buf[0] == 0)
	{
	printf("<TR><TD>No Comments</TD></TR>\n");
	}
else	while(buf[x] != 0)
	{
	for(x=y;buf[x] != '\n' && buf[x] != 0;x++);
	strncpy(s,&buf[y],x-y);
	s[x-y] = 0;
	y=x+1;
	if(strcmp(s,"<.>") == 0)
		{
		printf("<tr><td>\n");
		y++;
		for(x=y;buf[x] != '>';x++);
		strncpy(s,&buf[y],x-y);
		s[(x-y)] = 0;
		printf("Nick: <STRONG>%s</STRONG><BR>\n",s);
		y=x+3;
		
		for(x=y;buf[x] != '>';x++);
		strncpy(s,&buf[y],x-y);
		s[(x-y)] = 0;
		printf("Date: <STRONG>%s</STRONG><P>\n",s);
		y=x+2;
			
//			fprintf(stderr,"got here.\n");
		for(x=y;buf[x] != '>';x++);
		y=x+2;
		for(x=y;strncmp(&buf[x],"<end>",5) != 0 && buf[x] != 0;x++);
		strncpy(s,&buf[y],x-y);
		s[(x-y)] = 0;
		printf("%s\n",s);
		y=x;

//			fprintf(stderr,"got here too.\n");	

		for(x=y;buf[x] != '>';x++);
		y=x+2;
		
		printf("</td></tr>\n");	
		}
	else
		continue;
			
	}
//	fprintf(stderr,"%s\n",buf);
myerror("done with comments.");
printf("</TABLE><P><P><P>\n");
printf("Enter your comment here:<P>\n");
printf("<FORM action=\"/cgi-bin/addcomment.cgi\" method=\"POST\">\n");
printf("<input type=\"hidden\" NAME=\"h1\" VALUE=\"%s\">\n",commentname);
printf("Nick: <input type=\"text\" NAME=\"n1\" VALUE=\"%s\">\n",name);
printf("<P>Comment:<BR>\n");
printf("<TEXTAREA NAME=\"q1\" ROWS=6 COLS=60></TEXTAREA><P>\n");
printf("<input type=\"submit\" VALUE=\"SUBMIT\">\n");
printf("</FORM><P><P><P>\n");
printf("There are no specific posting guidelines... (yet)..  however, excessively obscene, abusive, harrassing, trollish, or posts with illegal content may be removed at the discretion of the site owner.\n"); 
printf("</TD></TR></TABLE>\n");
}


int	refcheck()
{
char	searchstr[2000];
if(getenv("HTTP_REFERER") != NULL)
	strcpy(searchstr,getenv("HTTP_REFERER"));
else
	return(0);
fprintf(stderr,"ref: .%s.\n",searchstr);
if(CIgrepit(searchstr,"drivemeinsane.com"))
	return(1);
if(CIgrepit(searchstr,"64.246.28.98"))
	return(1);
return(0);
}


int	loadcam(struct	camtype *cam, int record)
{
int	x,y,z;
int	size;
int	handle;

handle = open("/tmp/webcams.dat",O_RDONLY);
if(handle == -1)
	return(-1);
size = lseek(handle,0,2) / sizeof *cam;
if(record >= size)
	{
	close(handle);
	return(-1);
	}
lseek(handle,record * sizeof *cam,0);
read(handle,cam, sizeof *cam);
close(handle);
}

int	savecam(struct	camtype *cam, int record)
{
int	x,y,z;
int	size;
int	handle;

handle = open("/tmp/webcams.dat",O_RDONLY);
if(handle == -1)
	return(-1);
size = lseek(handle,0,2) / sizeof *cam;
if(record > size)
	{
	close(handle);
	return(-1);
	}
lseek(handle,record * sizeof *cam,0);
write(handle,cam, sizeof *cam);
close(handle);
}

int	adminname(char	*name)
{
FILE	*input;
char	s[256];

input = fopen("/home/pjm/adminnames.txt","r");
fgets(s,250,input);
s[250] = 0;
s[strlen(s)-1] = 0;
while(!feof(input))
	{
	if(strcmp(name,s) == 0 && s[0] != 0)
		{
		fclose(input);
		return(1);
		}

	fgets(s,250,input);
	s[250] = 0;
	s[strlen(s)-1] = 0;
	}
fclose(input);
return(0);
}

int	oldadminname(char	*name)
{
if(strcmp(name,"Restil") == 0)
	return(1);
if(strcmp(name,"Drew") == 0)
	return(1);
if(strcmp(name,"Gertie") == 0)
	return(1);
if(strcmp(name,"Skeetz") == 0)
	return(1);
if(strcmp(name,"Frank") == 0)
	return(1);
if(strcmp(name,"Boop") == 0)
	return(1);
if(strcmp(name,"Dweasel") == 0)
	return(1);
return(0);
}


int	xmlcount(char *token, char *buffer)
{
int	x,y,z;
int	startpos;
int	counter;
int	tokensize;
int	tokensize2;
int	size;
char	searchtoken[256];
char	searchtoken2[256];
char	*retbuf;


counter=0;
sprintf(searchtoken,"<%s>",token);
size = strlen(buffer);
tokensize = strlen(searchtoken);
tokensize2 = strlen(searchtoken2);
for(z=0;z<size;z++)
	{
	if(strncmp(&buffer[z],searchtoken,tokensize) == 0)	// found a token
		{
		counter++;
		}
	}
return(counter);
}

stripfilter(char *buf, char *ret)
{
int	x,y,z;
int	size1,size2;
int	pos1,pos2;
int	blind;
char	tempstr[50000];

size1 = strlen(buf);
blind=0;
for(z=0;z<50000;z++)
	tempstr[z] = 0;
pos1=pos2=0;

for(pos1=0;pos1<size1;pos1++)
	{
	if(buf[pos1] == '\n')
		{
		strcat(tempstr,"<BR>");
		pos2 = strlen(tempstr);
		}
	else if(buf[pos1] == '<')
		blind=1;
	else if(buf[pos1] == '>' && blind)
		blind=0;
	else
		{
		if(!blind)
			tempstr[pos2++] = buf[pos1];
		}
	}
strcpy(ret,tempstr);
}

int     isdigit(char    c)
{
if(c >= '0' && c <= '9')
	return(1);
else
	if(c == '.')
		return(1);
else
	return(0);
}

int	httpgrab(char *host, int port, char *filename, int maxsize,char *buf)
{
int	x,y,z;
int	rval;
int	sock;
char	s[2000];

sock = connect_socket(host,port);
//fprintf(stderr,"Connected to %s on port %d.\n",host,port);
sprintf(s,"GET %s\n",filename);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writing on stream socket");
//fprintf(stderr,"Sent GET %s.\n",filename);
x = 0;	
do
	{
	y = 1460;
	if(maxsize - 1460 < x)
		y = maxsize - x;
//	fprintf(stderr,"x = %d   y = %d\n",x,y);
	if(( rval = read(sock, &buf[x], y)) < 0)
		perror("Reading from stream socket");
	x = x + rval;
//	fprintf(stderr,"Reading.  x=%d  rval=%d\n",x,rval);
	}
while(rval > 0);
//	fprintf(stderr,"Done reading. buf= .%s.\n",buf);	
close(sock);
buf[x] = 0;
return(x);
}

int	timedhttpgrab(char *host, int port, char *filename, int maxsize,char *buf,int timeout)
{
int	x,y,z;
int	rval;
int	sock;
long	starttime,t;
char	s[2000];
fd_set	rfds;
struct	timeval	tv;
int	retval;

//printf("Timed http grab.\n");

//printf("getting time.\n");
time(&starttime);
//fprintf(stderr,"calling timed_connect_socket.  http://%s:%d%s\n",host,port,filename);
sock = timed_connect_socket(host,port,timeout);
//sock = connect_socket(host,port);
if(sock == -1)
	return(-1);
FD_ZERO(&rfds);
FD_SET(sock,&rfds);
//fprintf(stderr,"tcs worked. sock=%d\n",sock);
//fprintf(stderr,"Connected to %s on port %d.\n",host,port);
//sprintf(s,"GET %s\n",filename);
sprintf(s,"GET %s HTTP/1.0\nUser-Agent: DMI Cam Harvester/0.1\nHost: %s:%d\nAccept: */*\nConnection: close\r\n\r\n",filename,host,port);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writing on stream socket");
//fprintf(stderr,"Sent GET %s.\n",filename);
x = 0;	
time(&t);
do
	{
	y = 1460;
	if(maxsize - 1460 < x)
		y = maxsize - x;
//	fprintf(stderr,"x = %d   y = %d\n",x,y);
		
	tv.tv_sec=timeout - (t-starttime);
	tv.tv_usec=0;	
	retval = select(sock+1, &rfds, NULL, NULL, &tv);
	if(retval)
		{
		if(( rval = read(sock, &buf[x], y)) < 0)
			perror("Reading from stream socket");
		x = x + rval;
		}

//	else
//		perror("select");
	time(&t);
	if(t-starttime > timeout)
		return(-1);
//	fprintf(stderr,"Reading.  x=%d  rval=%d\n",x,rval);
	}
while(rval > 0);
//	fprintf(stderr,"Done reading. buf= .%s.\n",buf);	
close(sock);
buf[x] = 0;
return(x);
}

/*
This new function will parse out the content length and content type and
return only the image data.  If data other than an image is detected, an
error will return
*/

int	newtimedhttpgrab(char *host, int port, char *filename, int maxsize,char *buf,int timeout,int *rc, int *is)
{
int	x,y,z;
int	x2;
int	foundheaders;
int	connection;
int	contentlength;
int	contenttype;
int	i,j,k;
int	rval;
int	sock;
long	starttime,t;
char	s[2000];
fd_set	rfds;
struct	timeval	tv;
int	retval;
int	returncode;
int	imagestart;

//printf("Timed http grab.\n");

//printf("getting time.\n");
time(&starttime);
//fprintf(stderr,"calling timed_connect_socket.  http://%s:%d%s\n",host,port,filename);
sock = timed_connect_socket(host,port,timeout);
//sock = connect_socket(host,port);
if(sock == -1)
	return(-1);
FD_ZERO(&rfds);
FD_SET(sock,&rfds);
//fprintf(stderr,"tcs worked. sock=%d\n",sock);
//fprintf(stderr,"Connected to %s on port %d.\n",host,port);
//sprintf(s,"GET %s\n",filename);
sprintf(s,"GET %s HTTP/1.0\nUser-Agent: DMI Cam Harvester/0.1\nHost: %s:%d\nAccept: */*\nConnection: close\r\n\r\n",filename,host,port);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writing on stream socket");
//fprintf(stderr,"Sent GET %s.\n",filename);
x = 0;	
time(&t);
foundheaders=0;
connection=0;
contentlength=0;
contenttype=0;
imagestart=0;
do
	{
	y = 1460;
	if(maxsize - 1460 < x)
		y = maxsize - x;
//	fprintf(stderr,"x = %d   y = %d\n",x,y);
		
	if(connection == 1 && (x - imagestart) >= contentlength)
		break;
	tv.tv_sec=timeout - (t-starttime);
	tv.tv_usec=0;	
	retval = select(sock+1, &rfds, NULL, NULL, &tv);
	if(retval)
		{
		if(( rval = read(sock, &buf[x], y)) < 0)
			perror("Reading from stream socket");

		x2=0;
		if(x == 0)
			{
			if(strncmp(buf,"HTTP",4) == 0)
				{
				foundheaders=1;
				for(i=0;i<rval && buf[i] != ' ';i++)
					{
					if(i< rval)
						returncode = atoi(&buf[i+1]);
					else
						returncode = 404;
					}
				printf("Host: %s  Return code: %d\n",host,returncode);
				for(i=0;i<rval;i++)
					{
					if(buf[i] == '\n' && (buf[i+1] == '\n' || buf[i+1] == 13))
						{
						imagestart=i;
						while(buf[imagestart] == 10 || buf[imagestart] == 13)
						imagestart++;
						break;
						}
					}
				printf("Host: %s  imagestart: %d\n",host,imagestart);
				}
			else if(strncmp(buf,"Content-Type:",13) == 0 || strncmp(buf,"Server:",7) == 0)
				{
				foundheaders=1;
				for(i=0;i<rval;i++)
					{
					if(buf[i] == '\n' && (buf[i+1] == '\n' || buf[i+1] == 13))
						{
						imagestart=i;
						while(buf[imagestart] == 10 || buf[imagestart] == 13)
							imagestart++;
						break;
						}
					}
				returncode = 200;
				}
			else
				returncode = 200;
			if(foundheaders)
				{
				contentlength = 0;
				// first check to see the connection type.
				for(i=0;i<imagestart;i++)
					{
					if(strncmp(&buf[i],"Connection: ",12) == 0)
						{
						if(strncmp(&buf[i+12],"keep-alive",10) == 0 || strncmp(&buf[i+12],"Keep-alive",10) == 0)
							connection = 1;
						else
							connection = 0;
						printf("host: %s   connection: %d\n",host,connection);
						}
				// next look for the content length
					if(strncmp(&buf[i],"Content-Length: ",16) == 0)
						{
						contentlength = atoi(&buf[i+16]);
						printf("host: %s   contentlength = %d\n",host,contentlength);
						}
					if(strncmp(&buf[i],"Content-Type: ",14) == 0)
						{
						if(strncmp(&buf[i+14],"image/jpeg",10) == 0)
							contenttype=1;
						else
							contenttype=0;
						printf("host: %s   Contenttype = %d\n",host,contenttype);
						}
					}
				}
			}
		else
			x2 = x;

		x = x + rval;
		}

//	else
//		perror("select");
	time(&t);
	if(t-starttime > timeout)
		return(-1);
//	fprintf(stderr,"Reading.  x=%d  rval=%d\n",x,rval);
	}
while(rval > 0);
//	fprintf(stderr,"Done reading. buf= .%s.\n",buf);	
close(sock);
buf[x] = 0;
*rc = returncode;
*is = imagestart;
return(x);
}

int	httprelaygrab(char *host, int port, char *filename, int maxsize,char *buf, FILE *output)
{
int	x,y,z;
int	x2;
int	rval;
int	sock;
char	s[2000];

sock = connect_socket(host,port);
//fprintf(stderr,"Connected to %s on port %d.\n",host,port);
sprintf(s,"GET %s HTTP/1.0\nUser-Agent: DMI Cam Harvester/0.1\nHost: %s:%d\nAccept: */*\nConnection: close\n\n",filename,host,port);
if((write(sock, s, strlen(s)), 0) < 0)
	perror("writing on stream socket");
fprintf(output,"Content-type: image/jpeg\n\n");
//fprintf(stderr,"Sent GET %s.\n",filename);
x = 0;	
do
	{
	y = 1460;
	if(maxsize - 1460 < x)
		y = maxsize - x;
//	fprintf(stderr,"x = %d   y = %d\n",x,y);
	if(( rval = read(sock, &buf[x], y)) < 0)
		perror("Reading from stream socket");
	x2=0;
	if(x == 0)
		{
		if(strncmp(buf,"HTTP",4) == 0)
			{
			for(z=0;z<rval;z++)
				{
				if(buf[z] == '\n' && buf[z+1] == '\n')
					{
					x2 = z+2;
					break;
					}
				}
			}
		}
	else
		x2 = x;
	for(z=x2;z<x+rval;z++)
		{
		putc(buf[z],stdout);
		}

	x = x + rval;
//	fprintf(stderr,"Reading.  x=%d  rval=%d\n",x,rval);
	}
while(rval > 0);
//	fprintf(stderr,"Done reading. buf= .%s.\n",buf);	
close(sock);
buf[x] = 0;
return(x);
}

int	copyfile(char *srcfile, char *destfile)
{
int	x,y,z;
int	size;
int	handle;
int	handle2;
char	*buf;

handle = open(srcfile,O_RDONLY);
size = lseek(handle,0,2);

handle2 = open(destfile,O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
buf = malloc(size);
lseek(handle,0,0);
read(handle,buf,size);
close(handle);
lseek(handle2,0,0);
write(handle2,buf,size);
close(handle2);
free(buf);
}

add_cam_message(int camnum,char *buf)
{
int	x,y,z;
long	t;
int	pos;
int	size;
int	handle;
FILE	*output;
char	filename[256];
struct	camtype	cam;
struct	tm	*tim;

time(&t);
tim = localtime(&t);
getcam(camnum,&cam);
sprintf(filename,"/extreme/cam_messages/%s",cam.name);
output = fopen(filename,"a+");
fprintf(output,"[%2.2d/%2.2d/%2.2d - %2.2d:%2.2d:%2.2d] %s\n",tim->tm_mon+1,tim->tm_mday,tim->tm_year%100,tim->tm_hour,tim->tm_min,tim->tm_sec,buf);
fclose(output);
chmod(filename,S_IRWXU | S_IRWXG | S_IRWXO);
}

display_last_cam_message(int camnum)
{
int	x,y,z;
long	t;
int	pos;
int	size;
int	handle;
char	c;
FILE	*output;
char	filename[256];
char	s[2000];
struct	camtype	cam;
struct	tm	*tim;

time(&t);
tim = localtime(&t);
getcam(camnum,&cam);
sprintf(filename,"/extreme/cam_messages/%s",cam.name);
handle = open(filename,O_RDONLY);
if(handle < 0)
	{
	printf("There are no messages for this cam.");
	return(-1);
	}
size = lseek(handle,0,2);
z = size-2;
do
	{
	lseek(handle,z,0);
	read(handle,&c,1);
	z--;
	}
while(z >= 0 && c != '\n');
z = z + 1;
close(handle);
output = fopen(filename,"r");
fseek(output,z,0);
fgets(s,1990,output);
fclose(output);
s[1990] = 0;
printf("%s",s);
}


/*
urlparser:  Take a url string and extract the relevant data from it.
*/


int	urlparser(char *url, struct urldatatype *urldata)
{
int	x,y,z;
char	s[2000];
int	pos;
int	tokenposition;


pos=0;
tokenposition = 0; // we're at the beginning.. no data obtained yet.
strcpy(urldata->pragma,"");
strcpy(urldata->host,"");
strcpy(urldata->user,"");
strcpy(urldata->password,"");
strcpy(urldata->filename,"");
strcpy(urldata->query,"");
strcpy(urldata->fragment,"");

// start parsing.  Searching for either a pragma or a host
if(CIngrepit(&url[pos],"http://",7))
	{
	strcpy(urldata->pragma,"http");
	pos=7;
	tokenposition=4;	// past pragma
	}
// at this point, we know we're searching for a host
for(z=pos;url[z] != 0 &&  url[z] != '?' && url[z] != ':' && url[z] != '/' && url[z] != '#';z++);
if(z > pos)
	{
	strncpy(urldata->host,&url[pos],z-pos);
	if(url[z] == 0)
		{
		// we're done
		return(1);
		}
	else if(url[z] == ':')
		tokenposition = 5;
	else if(url[z] == '/')
		tokenposition = 6;
	else if(url[z] == '?')
		tokenposition = 7;
	else if(url[z] == '#')
		tokenposition = 8;
	pos = z+1;
	}
// checking for port
if(tokenposition == 5)
	{
	urldata->port = atoi(&url[pos]);	
	for(z=pos;url[z] != 0 &&  url[z] != '?' && url[z] != '/' && url[z] != '#';z++);
	if(z > pos)
		{
		if(url[z] == 0)
			{
			// we're done
			return(1);
			}
		else if(url[z] == '/')
			tokenposition = 6;
		else if(url[z] == '?')
			tokenposition = 7;
		else if(url[z] == '#')
			tokenposition = 8;
		pos = z+1;
		}
	}
else
	{
	// no port provided, default to 80
	urldata->port = 80;
	}

// now searching for filename
if(tokenposition==6)
	{
	pos--;
	for(z=pos;url[z] != 0 &&  url[z] != '?' && url[z] != '#';z++);
	if(z > pos)
		{
		strncpy(urldata->filename,&url[pos],z-pos);
		if(url[z] == 0)
			{
			// we're done
			return(1);
			}
		else if(url[z] == '?')
			tokenposition = 7;
		else if(url[z] == '#')
			tokenposition = 8;
		pos = z+1;
		}

	}
if(tokenposition==7)
	{
	for(z=pos;url[z] != 0 &&  url[z] != '#';z++);
	if(z > pos)
		{
		strncpy(urldata->query,&url[pos],z-pos);
		if(url[z] == 0)
			{
			// we're done
			return(1);
			}
		else if(url[z] == '#')
			tokenposition = 8;
		pos = z+1;
		}

	}

if(tokenposition==8)
	{
	for(z=pos;url[z] != 0;z++);
	if(z > pos)
		{
		strncpy(urldata->fragment,&url[pos],z-pos);
		if(url[z] == 0)
			{
			// we're done
			return(1);
			}
		pos = z+1;
		}
	}
return(1);
}

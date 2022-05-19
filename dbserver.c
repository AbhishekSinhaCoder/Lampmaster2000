/*
This program has been written by Paul Mathis (restil@alignment.net).  
Please feel free to distribute or use it in any way you wish.  If you make
modifications, please let me know as I might find them useful for myself.

This is the server half of the doorbell program.  After the doorbell client
detects the ring, it sends this information to this server for processing.

This server accomplishes two primary goals, and may include others.

1 - to make a log of all doorbell events.

2 - to simulate an actual doorbell and make noise.  The easiest way to 
do this from a linux machine is to simply play a .wav file.

This server may be run on the same machine as the client.  They were
separated in the event that another machine had better capabilities to 
log files or play sounds.  It also frees up the client machine so it can
pay closer attention to the doorbell.

My primary purpose for this project was to hook my doorbell up to the 
internet so people could view logs of the rings.  Writing a simple CGI program
to access the log file completes the cycle of events required to do so.

Also, please realize that these programs are a work in progress and tend to
be a serious mess.  I plan to clean them up and rework them as time goes on,
and I'll probably make some effort to combine some of the different programs
into a single one, as well as add more error detection and security features.
Please check back frequently for updates.

Of course, the author hopes that this information will be useful, but 
no warranties are expressed or implied that it will do what it says it will
do, and no responsibilities will be accepted if this turns your computer into
a live nuclear bomb, yada yada yada..   Basically, don't blame me for 
anything. :)

*/





#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define DATA "You got it from me"

main(argc, argv)
int	argc;
char 	*argv[];
{
int	sock;
int	length;
struct	sockaddr_in server;
int	msgsock;
char	buf[1024];
char	s[256],t[1024];
char	fn[100];
char	datestr[30];
char	filename[30];
int	rval;
int	i;
time_t	lt;
struct	tm	*tim;
FILE	*output;

sock = socket(AF_INET, SOCK_STREAM, 0);
if(sock < 0)
	{
	perror("Opening steram socket");
	exit(1);
	}
server.sin_family = AF_INET;
server.sin_addr.s_addr = INADDR_ANY;
server.sin_port = htons((u_short )5200);
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
do
	{
	msgsock = accept(sock, 0,0);
	if(msgsock == -1)
		perror("accept");
	else do
		{
		bzero(buf, sizeof(buf));
		if((rval = read(msgsock, buf, 1024)) < 0)
			perror("Reading stream message");
		i = 0;
		if(rval != 0)
			{
			if(strcmp(buf,"DING") == 0)
				{
				int zz;
				int forkval;
				int status;
				/* We got a ring */
				time(&lt);
				tim = localtime(&lt);
// Fork this next line out.
				forkval = fork();
				if(forkval == 0)
					{
					system("/usr/local/bin/wavplay /home/pjm/flint03.wav > /dev/null");
					exit(0);
					}
/* Text log */
				sprintf(s,"echo Front Door Ring:  %2.2d/%2.2d/%2.2d    %2.2d:%2.2d:%2.2d >> /var/lib/httpd/cgi-bin/dblogs.txt",tim->tm_mon+1,tim->tm_mday,tim->tm_year % 100,tim->tm_hour,tim->tm_min,tim->tm_sec);
/* http log */					
				sprintf(filename,"doorbell%3.3d%2.2d%2.2d%2.2d",tim->tm_yday,tim->tm_hour,tim->tm_min,tim->tm_sec);
				system("/var/lib/httpd/cgi-bin/dodoor");
				sprintf(s,"mkdir /new/doorlogs4/%s",filename);
				system(s);
				for(zz=0;zz<20;zz++) 
					{
					sprintf(s,"mv /tmp/door%2.2d.jpg /new/doorlogs4/%s/%s-%2.2d.jpg",zz,filename,filename,zz);
					system(s);
					}
				sprintf(s,"echo \"<HTML><BODY><CENTER><H1>Doorbell Logs For %2.2d/%2.2d/%2.2d at %2.2d:%2.2d:%2.2d</H1><P>\" > /new/doorlogs4/%s/head.txt",tim->tm_mon+1,tim->tm_mday,tim->tm_year % 100,tim->tm_hour,tim->tm_min,tim->tm_sec,filename);
				system(s);
				sprintf(s,"cp /new/doorlogs3/tail.txt /new/doorlogs4/%s/tail.txt",filename);
				system(s);
				sprintf(s,"cp /new/doorlogs4/%s/%s-00.jpg /new/doorlogs4",filename,filename);
				system(s);
				sprintf(s,"%2.2d/%2.2d/%2.2d  %2.2d:%2.2d:%2.2d<BR><A HREF=\"/doorlogs/%s/pics.html\">More Pics</A><BR>Identified As:<BR><A HREF=\"people/unk.jpg\">UNKNOWN</A>\">\n",tim->tm_mon+1,tim->tm_mday,tim->tm_year % 100,tim->tm_hour,tim->tm_min,tim->tm_sec,filename);
				sprintf(fn,"/new/doorlogs4/%s-00.jpg.txt",filename);
				output = fopen(fn,"a");
				
				fprintf(output,s);	
				fclose(output);
				sprintf(s,"cd /new/doorlogs4/%s; /var/lib/httpd/htdocs/people/test/dopicpage",filename);
				system(s);
				sprintf(s,"cd /new/doorlogs4; /var/lib/httpd/htdocs/captures/dopicpage",filename);
				system(s);
				wait(&status);
				if(write(msgsock, DATA, sizeof(DATA)), 0)
					perror("Writting on stream socket ");
				}
			if(strcmp(buf,"MOVE") == 0)
				{
				int zz;
				int forkval;
				int status;
				/* We got a ring */
				time(&lt);
				tim = localtime(&lt);
// Fork this next line out.
				forkval=fork();
				if(forkval == 0)
					{
					system("/usr/local/bin/wavplay /home/pjm/alert.wav > /dev/null");
					exit(0);
					}
/* Text log */
/* http log */					
				sprintf(filename,"movedoorbell%3.3d%2.2d%2.2d%2.2d",tim->tm_yday,tim->tm_hour,tim->tm_min,tim->tm_sec);
				system("/var/lib/httpd/cgi-bin/domove");
				sprintf(s,"mkdir /new/movedoorlogs2/%s",filename);
				system(s);
				for(zz=0;zz<2;zz++) 
					{
					sprintf(s,"mv /tmp/move%2.2d.jpg /new/movedoorlogs2/%s/%s-%2.2d.jpg",zz,filename,filename,zz);
					system(s);
					}
				sprintf(s,"echo \"<HTML><BODY><CENTER><H1>Motion Sensor Logs For %2.2d/%2.2d/%2.2d at %2.2d:%2.2d:%2.2d</H1><P>\" > /new/movedoorlogs2/%s/head.txt",tim->tm_mon+1,tim->tm_mday,tim->tm_year % 100,tim->tm_hour,tim->tm_min,tim->tm_sec,filename);
				system(s);
				sprintf(s,"cp /new/movedoorlogs/tail.txt /new/movedoorlogs2/%s/tail.txt",filename);
				system(s);
				sprintf(s,"cp /new/movedoorlogs2/%s/%s-00.jpg /new/movedoorlogs2",filename,filename);
				system(s);
				sprintf(s,"%2.2d/%2.2d/%2.2d  %2.2d:%2.2d:%2.2d<BR><A HREF=\"/movedoorlogs/%s/pics.html\">More Pics</A><BR>Identified As:<BR><A HREF=\"people/unk.jpg\">UNKNOWN</A>\">\n",tim->tm_mon+1,tim->tm_mday,tim->tm_year % 100,tim->tm_hour,tim->tm_min,tim->tm_sec,filename);
				sprintf(fn,"/new/movedoorlogs2/%s-00.jpg.txt",filename);
				output = fopen(fn,"a");
				
				fprintf(output,s);	
				fclose(output);
				sprintf(s,"cd /new/movedoorlogs2/%s; /var/lib/httpd/htdocs/people/test/dopicpage",filename);
				system(s);
				sprintf(s,"cd /new/movedoorlogs2; /var/lib/httpd/htdocs/captures/dopicpage",filename);
				system(s);
				wait(&status);
				if(write(msgsock, DATA, sizeof(DATA)), 0)
					perror("Writting on stream socket ");

				}
			if(strcmp(buf,"OPEN") == 0)
				{
				/* The door opened */
				time(&lt);
				tim = localtime(&lt);
				sprintf(s,"echo Door Opens:  %2.2d/%2.2d/%2.2d    %2.2d:%2.2d:%2.2d >> /var/lib/httpd/cgi-bin/doorlogs.txt",tim->tm_mon+1,tim->tm_mday,tim->tm_year % 100,tim->tm_hour,tim->tm_min,tim->tm_sec);
				system(s);
				system("/usr/local/bin/wavplay /home/pjm/flint03.wav > /dev/null");
/*
				output = fopen("/var/lib/httpd/cgi-bin/dblogs.txt","a");
				
				fprintf(output,s);	
				fclose(output);
*/
				if(write(msgsock, DATA, sizeof(DATA)), 0)
					perror("Writting on stream socket ");
				}
			if(strcmp(buf,"CLOSE") == 0)
				{
				/* The door was closed */
				time(&lt);
				tim = localtime(&lt);
				sprintf(s,"echo Door Closes:  %2.2d/%2.2d/%2.2d    %2.2d:%2.2d:%2.2d >> /var/lib/httpd/cgi-bin/doorlogs.txt",tim->tm_mon+1,tim->tm_mday,tim->tm_year % 100,tim->tm_hour,tim->tm_min,tim->tm_sec);
				system(s);
				system("/usr/local/bin/wavplay /home/pjm/flint03.wav > /dev/null");
/*
				output = fopen("/var/lib/httpd/cgi-bin/dblogs.txt","a");
				
				fprintf(output,s);	
				fclose(output);
*/
				if(write(msgsock, DATA, sizeof(DATA)), 0)
					perror("Writting on stream socket ");
				}
			}
		} while (rval != 0);
	close(msgsock);
	} while (1);
}
	







/*

This is the client program for the switch.  It sends a message to a server
with the command "ON" or "OFF".  

The client program isn't very special.  It is simply designed in this case
to be run as a cgi script from a webpage, although it can be activated 
from a command line as well.

As with my doorbell program, I have designed the client and server separately
and allow them to work over a TCP/IP network.  This provides the greatest
amount of flexibility, as it is generally unwise to hook up experimental
devices to production machines.  I use a $30 486/66 to operate the switch,
and that is the total risk of this project as a result.  

This program takes as arguments, the IP address of the switch server, 
the port number to use, and the command.  

If you have questions about this, or any of my other programs, feel free to
contact me at: restil@alignment.net

*/







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
int	temp;




temp = 0; 	/* 0 if currently NOT pressed, 1 is currently pressed */




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
	
	if((connect(sock, &server, sizeof(server)), 0) < 0)
		{
		perror("connecting stream socket");
		exit(1);
		}
	strcpy(s,"");
	
	printf("argv3: %s argc: %d\n",argv[3], argc);
	strcat(s,argv[3]);
	for(x=4;x<argc;x++)
		{
		strcat(s," ");
		strcat(s,argv[x]);
		}
	printf("s: .%s.\n",s);
	
	
	
	if((write(sock, s, strlen(s)), 0) < 0)
		perror("writing on stream socket");
	
	if(( rval = read(sock, buf, 1024)) < 0)
		perror("Reading from stream socket");
		
	close(sock);

}









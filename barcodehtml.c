#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

struct	upc_type
	{
	char	upc[30];
	char	name[80];
	int	quantity;
	int	type;	// 1:consumable 2:appliance
	float	cost;
	int	total_consumed;	//how many of these have we discarded.
	struct		//for adjustable rate products
		{
		int	static_pos;
		int	price_start;
		int	price_end;
		}specials; 
	int	unit;	// size of product.
	char	unit_type[15];	// unit measurement (oz,cans, etc)
	int	parent;	// parent product (or -1 if none)
	struct		// children products
		{
		int	number;
		int	quantity;
		}children[10];
	int	parent_type;
	};


struct	upc_key_element
	{
	char	upc[30];
	int	number;
	};

main()
{
int	x,y,z;
int	handle;
int	size;
struct	upc_type	upcdata;


handle = open("/home/pjm/barcode.dat",O_RDONLY | O_CREAT);
size = lseek(handle,0,2) / sizeof upcdata;
close(handle);

printf("Content-type: text/html\n");
printf("\n");
printf("<HTML>\n");
printf("<HEAD><TITLE>Generated Inventory Listing</TITLE></HEAD>\n");
printf("<BODY>\n");
printf("<FORM ACTION=\"/cgi-bin/barcodetrack.cgi\" METHOD=GET>\n");
printf("<TABLE BORDER=1>\n");
printf("	<TR><TH>Track</TH><TH>Number</TH><TH>Name</TH><TH>UPC Code</TH><TH>Quantity</TH></TR>\n");

for(z=0;z<size;z++)	
	{
	printf("<TR>");
	load_upc(&upcdata,z);
	printf("<TD><input TYPE=\"submit\" NAME=a%d VALUE=\"Track\" width=100%></TD>",z);
	printf("<TD>%d</TD>",z);
	printf("<TD>%s</TD><TD>%s</TD><TD>%d</TD>",upcdata.name,upcdata.upc,upcdata.quantity);
	printf("</TR>\n");
	}
printf("</TABLE>\n");
printf("</BODY></HTML>\n");
}



int	load_upc(struct	upc_type *upc, int number)
{
int	x,y,z;
int	handle;

handle = open("/home/pjm/barcode.dat",O_RDONLY | O_CREAT);
z = lseek(handle,0,2) / sizeof *upc;
if(number >= z)
	{
	printf("This number is too large.\n");
	return(-1);
	}
lseek(handle,number*sizeof *upc,0);
read(handle,upc,sizeof *upc);
close(handle);
}

int	save_upc(struct upc_type *upc, int number,int mode)
{
int	x,y,z;
int	handle;

handle = open("/home/pjm/barcode.dat",O_WRONLY | O_CREAT);
z = lseek(handle,0,2) / sizeof *upc;
if(mode)	// new
	{
	lseek(handle,0,2);
	number=z;
	}
else
	{
	if(number >= z)
		{
		printf("This number is too large.\n");
		return(-1);
		}
	lseek(handle,number*sizeof *upc,0);
	}
write(handle,upc,sizeof *upc);
close(handle);
return(number);
}

load_keys(struct upc_key_element *keys)
{
}

int	find_upc(char	upc[30])
{
int	x,y,z;
int	handle;
struct	upc_type	upcdata;

handle = open("/home/pjm/barcode.dat",O_RDONLY | O_CREAT);
z = lseek(handle,0,2) / sizeof upcdata;
close(handle);
for(x=0;x<z;x++)
	{
	y=load_upc(&upcdata,x);
	if(y== -1)
		{
		printf("find_upc error.\n");
		exit(1);
		}
	if(strcmp(upcdata.upc,upc)==0)
		return(x);
	}
return(-1);
}

edit_upc(int	number,int mode,char	upc[30])
{
int	x,y,z;
int	entry;
char	s[256];
struct	upc_type	upcdata;

if(!mode)
	{
	y=load_upc(&upcdata,number);
	if(y == -1)
		{
		printf("edit_upc error.\n");
		exit(1);
		}
	}
else
	{
	strcpy(upcdata.upc,upc);
	upcdata.quantity=1;
	strcpy(upcdata.name,"");
	}
printf("UPC (%s): ",upcdata.upc);
gets(s);
if(s[0] != 0)
	{
	strcpy(upcdata.upc,s);
	}
printf("Quantity (%d): ",upcdata.quantity);
gets(s);
if(s[0] != 0)
	{
	upcdata.quantity = atoi(s);
	}
printf("Name (%s): ",upcdata.name);
gets(s);
if(s[0] != 0)
	{
	strcpy(upcdata.name,s);
	}
save_upc(&upcdata,number,mode);
}


scanproc()
{
int	x,y,z;
char	sm[10][10];
int	number;
int	scanmode;
struct	upc_type upcdata;
char	s[1000];

strcpy(sm[1],"add");
strcpy(sm[2],"discard");
strcpy(sm[3],"edit");
scanmode=1;

while(1)
	{
	printf("Current scan mode: %s\n",sm[scanmode]);
	printf("(a/d/e/q) Barcode: ");
	gets(s);
	if(s[0] == 'a')
		{
		scanmode=1;
		continue;
		}
	if(s[0]=='d')
		{
		scanmode=2;
		continue;
		}
	if(s[0]=='e')
		{
		scanmode=3;
		continue;
		}
	if(s[0] == 'q')
		{
		break;
		}
	number=find_upc(s);
	if(number == -1)
		{
		edit_upc(0,1,s);
		continue;
		}
	y=load_upc(&upcdata,number);
	if(y == -1)
		{
		printf("Scanproc error.\n");
		exit(1);
		}
	printf("Found %s upc (%s) at position %d with quantity %d.\n",s,upcdata.name,number,upcdata.quantity);
	if(scanmode==1)
		upcdata.quantity++;
	else if(scanmode == 2)
		{
		upcdata.quantity--;
		if(upcdata.quantity==0)
			printf("You now have none left.  Time to restock.\n");
		if(upcdata.quantity<0)
			{
			printf("You now show having less than 0 of this item.  Value has been reset to 0.\n");
			upcdata.quantity=0;
			}
		}
	else if(scanmode == 3)
		{
		edit_upc(number,0,NULL);
		continue;
		}
	save_upc(&upcdata,number,0);
	}
}

/*
Barcode scanning program.

Stage 1: Complete
Get the damn thing working.  Scan barcodes and be able to edit, add, and 
discard items.  Preliminary support for parent/children.  Full database in
place, although not all fields are yet being used.  Inventory list available
via the web.  Non production phase.

Stage 2: 
Production phase.  Get the database setup finalized so no further projected
fields will be needed in the future.  Have all fields editable.  Transaction
logs in place.  Have inventory list, and various transaction reports
available via web.

Stage 3:
Convienence phase.  Full screen edit menu.  Obtain product names via the
barcode mysql database.  Support for dynamic UPC's (ground beef for instance).
Cost tracking.  Consumption tracking.  Automated shopping list creation
based upon current consumption rate.

Stage 4:
Automation phase.  Cookbook interface to automatically slate needed items
for selected recipies and cross referencing current inventory to create
a new shopping list.  Weekly meal planning, both selective and automatic.
Automated grocery ordering via groceryworks.com or the like.




TODO:

Add field for purchase date, number of days til expiration at time of purchase.



*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

struct	trans_type
	{
	int	number;
	int	mode;	//1 - add, 2 - discard
	float	cost;	// cost at the time of this transaction.
	long	tm;	// Timestamp
	};

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
	int	parent;	// parent product (or -1 if none) (-2 if has children)
	struct
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
int	size;
int	handle;
struct	upc_type	upcdata;
struct	trans_type	*trans;
struct	tm	*tim;
char	s[256];
int	total;
int	daytotal,monthtotal;
int	lastday,lastmonth;
int	hours[24];
char	days[7][20];
int	daytrack[7];
long	last20[20];
int	count;
float	last20rate;
long	leasttime;
long	greatesttime;
int	weekcount;
long	lastweek;
int	startcounting;

strcpy(days[0],"Sunday");
strcpy(days[1],"Monday");
strcpy(days[2],"Tuesday");
strcpy(days[3],"Wednesday");
strcpy(days[4],"Thursday");
strcpy(days[5],"Friday");
strcpy(days[6],"Saturday");
weekcount=0;
lastweek = time(NULL) - (60*60*24*7);
for(z=0;z<20;z++)
	last20[z]=0;
printf("Number to track: ");
//gets(s);
printf("Content-type: text/plain\n");
printf("\n");
strcpy(s,getenv("QUERY_STRING"));
printf("%s\n",s);
x = atoi(&s[1]);
printf("Tracking item #%d\n",x);
total=0;
daytotal=0;
monthtotal=0;
lastday=0;
lastmonth=0;
startcounting=0;
for(z=0;z<24;z++)
	hours[z] = 0;
for(z=0;z<7;z++)
	daytrack[z]=0;
handle=open("/home/pjm/barcodetrans.dat",O_RDONLY);
size=lseek(handle,0,2) / sizeof trans[0];

trans = malloc(sizeof trans[0] * size);
lseek(handle,0,0);
read(handle,trans,sizeof trans[0] * size);		

for(z=0;z<size;z++)
	{
	tim = localtime(&trans[z].tm);
	if(lastday != tim->tm_mday || lastmonth != tim->tm_mon+1)
		{
		lastday=tim->tm_mday;
//		if(startcounting)
			daytotal++;
		}
	if(lastmonth != tim->tm_mon+1)
		{
		lastmonth=tim->tm_mon+1;
//		if(startcounting)
			monthtotal++;
		}
	if(trans[z].number != x)
		continue;
	startcounting=1;
	load_upc(&upcdata,trans[z].number);	
	printf("Transaction #%d: Product (%s) was ",z,upcdata.name);
	if(trans[z].mode == 1)
		printf("added ");
	else
		{
		printf("discarded ");
		total++;
		last20[total % 20] = trans[z].tm;
		hours[tim->tm_hour]++;
		daytrack[tim->tm_wday]++;
		if(trans[z].tm > lastweek)
			weekcount++;
		}
	printf("at %2.2d:%2.2d:%2.2d on %2.2d/%2.2d/%d.\n",tim->tm_hour,tim->tm_min,tim->tm_sec,tim->tm_mon+1,tim->tm_mday,tim->tm_year+1900); 
	}
load_upc(&upcdata,x);	
printf("A total of %d (%s) have been consumed so far.\n",total,upcdata.name);
printf("The daily consumption average for this product is %0.1f.\n",(float)total/daytotal);
printf("The monthly consumption average for this product is %d.\n",total/monthtotal);
if(total > 20)
	count=20;
else
	count=total;
y=0;
for(z=1;z<count;z++)
	{
	if(last20[y] > last20[z])
		y=z;
	}
leasttime=y;
y=0;
for(z=1;z<count;z++)
	{
	if(last20[y] < last20[z])
		y=z;
	}
greatesttime=y;
last20rate = (float)(60*60*24*20)/(last20[greatesttime] - last20[leasttime]);
printf("The adjusted daily consumption rate for the last 20 transactions is %0.1f\n",last20rate);
printf("The adjusted daily consumption rate for the last week is %0.1f\n",(float)weekcount/7);
y=0;
for(z=1;z<24;z++)
	if(hours[z] > hours[y])
		y=z;
	
printf("The most common hour to consume this product is %2.2d:00.\n",y);
y=0;
for(z=1;z<7;z++)
	if(daytrack[z] > daytrack[y])
		y=z;
printf("The most common day to consume this product is %s.\n",days[y]);
printf("At the current rate of consumption, this product will run out in %0.1f days.\n",(float)upcdata.quantity*daytotal/(float)total);
printf("Paul's addiction to this product will cost him approximately $%1.2f this month.\n",(float)total*upcdata.cost*30.4/(float)daytotal);
close(handle);
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

handle = open("barcode.dat",O_WRONLY | O_CREAT);
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

handle = open("barcode.dat",O_RDONLY | O_CREAT);
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
	upcdata.parent = -1;
	upcdata.children[0].number=0;
	upcdata.children[0].quantity=0;
	upcdata.total_consumed=0;
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
printf("Parent (%d): ",upcdata.parent);
gets(s);
if(s[0] != 0)
	{
	upcdata.parent=atoi(s);
	}
if(upcdata.parent == -2)
	{
	printf("Number (%d): ",upcdata.children[0].number);
	gets(s);
	if(s[0] != 0)
		{
		upcdata.children[0].number = atoi(s);
		}
	printf("Quantity (%d): ",upcdata.children[0].quantity);
	gets(s);
	if(s[0] != 0)
		{
		upcdata.children[0].quantity = atoi(s);
		}
	}
printf("Cost (%f): ",upcdata.cost);
gets(s);
if(s[0] != 0)
	{
	upcdata.cost=atof(s);
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
struct	upc_type upcdata2;
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
		number=find_upc(s);
		load_upc(&upcdata,number);
		save_trans(number,scanmode,upcdata.cost);
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
		{
		if(upcdata.parent != -2)
			{
			upcdata.quantity++;
			save_trans(number,scanmode,upcdata.cost);
			}
		else
			{
			y=load_upc(&upcdata2,upcdata.children[0].number);
			if(y == -1)
				{
				printf("Scanproc error.\n");
				exit(1);
				}
			upcdata2.quantity += upcdata.children[0].quantity;
			for(z=0;z<upcdata.children[0].quantity;z++)
				save_trans(upcdata.children[0].number,scanmode,upcdata.cost);	
			save_upc(&upcdata2,upcdata.children[0].number,0);
			upcdata.total_consumed++;
			
			}
		}
	else if(scanmode == 2)
		{
		upcdata.quantity--;
		upcdata.total_consumed++;
		save_trans(number,scanmode,upcdata.cost);
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


int	save_trans(int number,int mode,float	cost)
{
int	x,y,z;
int	handle;
struct	trans_type	trans;

trans.number=number;
trans.mode	= mode;
trans.cost=cost;
trans.tm = time(NULL);
handle = open("barcodetrans.dat",O_WRONLY | O_CREAT);
z = lseek(handle,0,2);
write(handle,&trans,sizeof trans);
close(handle);
}

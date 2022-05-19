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


Categories are groups of products that can effectively replace each other.
For instance, I may choose to purchase Dr. Pepper cans or Dr. Pepper
in 2/3 litre bottles.  In the end, all these products are Dr. Pepper.  In 
addition, I might replace one brand of soap for another.  While I may have
a soap preference, for the purposes of tracking soap consumption, one bar
is pretty much like another.  

The automated system will decide I need to purchase the equivalent of 24 12
oz cans of Dr. Pepper, but may only have a coupon for the bottles, so it can
determine that my best bet for savings will be to purchase the bottles 
instead.  Of course, since the program has no idea what the prices in the
store actually are, the consumer can purchase whatever makes sense at the time,
but at least he'll have a listing in front of him to work with.  If one product
is on sale, while the computer recommended another, appropriate changes
can be made at the time.



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

struct	recipe_type
	{
	char	name[80];	// Name of this recipe
	struct	
		{
		int	number;	// Number of this ingredient
		int	category;	// Category of this ingredient
		int	units;	// Number of units required.
		}ingredients[30];
	char	steps[160][20]; // instructions (20 lines)
	};

struct	upc_type
	{
	char	upc[30];
	char	name[80];
	int	quantity;
	int	type;	// 1:consumable 2:appliance 3: category
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
//	int	family;		// family code for coupons
//	int	category;	// category for this product
//	int	instant;	// Is this an instant consumable?
	};


struct	upc_key_element
	{
	char	upc[30];
	int	number;
	};

main()
{
scanproc();
}



int	load_upc(struct	upc_type *upc, int number)
{
int	x,y,z;
int	handle;

handle = open("barcode.dat",O_RDONLY | O_CREAT);
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

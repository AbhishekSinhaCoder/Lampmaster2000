#define LAMPSERVER_HOST	"doorbell"
#define	COOKIESERVER_HOST	"doorbell"
#define COMMENTSERVER_HOST	"doorbell"

/*
cam types:
0 - default (IE)
1 - mozilla
2 - windows media player
3 - mpeg
4 - reserve
5 - reserve
6 - wmp with audio
7 - screen 1
8 - screen 2
9 - screen 3

*/

struct camdata
	{
	char	host[80];
	int	port;
	char	filename[80];
	int	available;
//	int	active;		//check each option.
//	int	verify;		// need to poll this cam option?
	};

struct	oldcamtype
	{
//	int	scheduling;
//	int	status;		// 0 - new, 1 - validated 2- delete
	int	active;
	int	available;
	int	static_addr;
	int	refresh_rate;	// 0 if fast as possible
	int	update_rate;
//new	int	message_destination;	// where to send messages.
//	int	message_token[40];
//	int	show_profile;
//	int	show_website;
//	int	adult;		// does this cam show up on adult pages?
//	int	adult_mode;	// does this cam only show up for adult users?
//	int	commercial;
//	int	forcecache;	// Use cache as default instead of IE.
//	int	autocache;	// # of viewers when cache is forced.
//	int	cacherate;	// updaterate while forcecached.
	int	archive;
	int	archive_rate;
	struct	camdata cd[10];
	char	name[40];
	char	account[40];
	char	password[20];
	int	type;	// 0 - normal.  1 - always default (slideshow, etc)
	int	popup;	// popup cam link?
	int	capture; // capture pic link?
	int	pan; // pan controls?
	int	tilt; // tilt controls?
	int	zoom; // zoom controls?
	int	message; // message function?
	int	lamps; // are there lamps on this cam?
	int	showarchive;	// show the archive link?
	int	private;	// in privacy mode?
	int	audio;
	char	audiourl[80];
//	char	audiohost[80];
//	int	audioport;
//	char	audioadminpass[80];
//	int	audiotype;	// 0 - icecast, 1 - shoutcast, 2 - ffserver
//	int	phone;
//	char	phoneurl[80];
	int	music;
	int	httpqport;
	char	httpqhost[120];
	int	localfiles;
//	long	creationdate;
//	long	lastupdate;
//	char	lampaccount[40];	// for matching up lamps.
//	int	autoactivate;		// do we keep trying to activate?
//	int	hostlocation;		// alternate ways to obtain the host
//	char	gethostscript[80];	// script to obtain host	
//	int	relaymode;		// do we relay the cam image?
//	int	relayrate;
//	int	alwayssethost;		// do we always set the host on activate?
	};	

/*
message destination:
	0	- chatroom
	1	- alternate channel
	2	- private message
	3	- notice
	4	- cookie popup
*/
struct	camtype
	{
	int	scheduling;
	int	status;		// 0 - new, 1 - validated 2- suspend 3- delete
	int	active;
	int	available;
	int	static_addr;
	int	refresh_rate;	// 0 if fast as possible
	int	update_rate;
	int	message_destination;	// where to send messages.
	char	message_token[40];
	int	show_profile;
	int	show_website;
	int	adult;		// does this cam show up on adult pages?
	int	adult_mode;	// does this cam only show up for adult users?
	int	commercial;
	int	forcecache;	// Use cache as default instead of IE.
	int	autocache;	// # of viewers when cache is forced.
	int	cacherate;	// updaterate while forcecached.
	int	archive;
	int	archive_rate;
	struct	camdata cd[10];
	char	name[40];
	char	account[40];
	char	password[20];
	int	type;	// 0 - normal.  1 - always default (slideshow, etc)
	int	popup;	// popup cam link?
	int	capture; // capture pic link?
	int	pan; // pan controls?
	int	tilt; // tilt controls?
	int	zoom; // zoom controls?
	int	message; // message function?
	int	lamps; // are there lamps on this cam?
	int	showarchive;	// show the archive link?
	int	private;	// in privacy mode?
	int	audio;
	char	audiourl[80];
	char	audiohost[80];
	int	audioport;
	char	audioadminpass[80];
	int	audiotype;	// 0 - icecast, 1 - shoutcast, 2 - ffserver, 3-webcamxp
	int	phone;
	char	phoneurl[80];
	int	music;
	int	httpqport;
	char	httpqhost[120];
	int	localfiles;
	long	creationdate;
	long	lastupdate;
	char	lampaccount[40];	// for matching up lamps.
//	int	catagory;
//	char	watcherstring[80];
//	int	privatearchiverate;	//0 if off
//	int	defaultcam;		// IE is default;	
//	int	maxarchives;		//0 for no max.
//	int	autoactivate;
//	char	hostmask[80];
	};	

struct	lampdisplaytype
	{
	char	name[20];
	char	label[40];
	int	icon;
	char	camname[80];
	};


struct	sklink
	{
	int	msgsock;
	long	connecttime;
	char	host[40];
	struct	sklink 	*next;
	};

struct	xmllinktype
	{
	char	*token;
	char	*data;
	char	*next;
	};


/*
Cam Schedules:
Activity:
	0	- do nothing.
	1	- activate cam.
	2	- deactivate cam.
	3	- put cam on private.
	4	- take cam off private.
	5	- activate screen.
	6	- deactivate screen.
	7 	- activate audio.
	8 	- deactivate audio.
	9	- Start archiving.
	10	- Stop Archiving.
	
*/

struct	camschedtype
	{
	long	action_time;	// what time of day does this take place.
	int	activity;
	};


/*
Project Status:
	0	- Submitted
	1	- Processing
	2	- Pending for more research
	3	- Approved
	4	- Work in Process
	5	- Work in Process, currently on hold
	6	- Completed
	7	- Partially completed, but finished.
	8	- Refused
	9	- Deleted
*/

struct	oldprojecttype
	{
	char	name[256];	
	int	status;		
	int	type;
	char	desc[3000];
	int	laborhours;	// hours doing actual work
	int	rdhours;	// hours doing R&D
	int	workedhours;	// hours spent on project so far.
	//int	softwarehours;
	float	materialcost;	// cost of materials
	float	hourlyrate;	// how much is labor worth
	char	submitaccount[80];	// who submitted
	char	workaccount[80];	// who worked on it.
	int	priority;
	int	complexity;
//	int	subproject;	// does this project have a subproject?
//	char	subprojectname[80];	//name of the subproject.
	};

struct	projecttype
	{
	char	name[256];	
	int	status;		
	int	type;
	char	desc[3000];
	int	laborhours;	// hours doing actual work
	int	rdhours;	// hours doing R&D
	int	workedhours;	// hours spent on project so far.
	int	softwarehours;
	float	materialcost;	// cost of materials
	float	hourlyrate;	// how much is labor worth
	char	submitaccount[80];	// who submitted
	char	workaccount[80];	// who worked on it.
	int	priority;
	int	complexity;
	int	subproject;	// does this project have a subproject?
	char	subprojectname[80];	//name of the subproject.
	};


struct  truthtype
        {
        int     gender; //0 - either, 1 - male, 2- female
        char    *truthtext;     // the truth
        int     rating;
        };

struct  daretype
        {
        int     gender;
        int     rating;
        char    *daretext;
        int     prefmask;
        };

struct	camcommandtype
	{
	int	camnum;
	int	cookienumber;
	int	mask;
	char	command[80];
	char	account[80];
	char	password[80];
	char	cookiename[80];
	char	host[20];
	struct	camtype	cam;
	};

struct	image_overlay_type
	{
	char	filename[256];
	struct	
		{
		int	x;
		int	y;
		char	text[160];
		};
	};

struct	urldatatype
	{
	char	pragma[20];
	char	host[256];
	char	user[80];
	char	password[80];
	int	port;
	char	filename[256];
	char	query[256];
	char	fragment[256];
	};


struct	zipdatatype
	{
	char	zipcode[10];
	char	lat[15];
	char	longitude[15];
	char	zipclass;
	char	poname[30];
	char	state[5];
	char	county[5];
	char	idmarplot[20];
	};

struct	commentdatatype
	{
	char 	*subject;
	long	postdate;
	char	account[80];
	char	nick[80];
	char	ip[20];	
	char	host[256];
	char	*comment;
	};

struct	newstype
	{
	long	postdate;
	char	account[80];
	char	newscategory[80];
	char	*newstitle;
	char	*newstext;
	};

struct	capturetype
	{
	char	imagefilename[80];
	char	*caption;
	char	ip[20];
	char	host[256];
	char	*cookiename;
	char	*accountname;
	char	*camname;
	long	capdate;
	};

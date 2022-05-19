/*
This header file contains the structures for the visitors page.
*/

struct	webpagetype
	{
	char	title[80];
	char	url[256];
	/* Future features
	char	type[40];
*/
	};


struct	visitortype
	{
	char	name[80];
	char	nick[80];
	int	age;
	long	birthdate;
	char	gender[10];
	char	status[40];
	struct	webpagetype	webpage[5];
	struct	webpagetype	webcam[2];
	char	webcampicurl[256];
	char	IRCname[40];
	char	AIMname[40];
	char	YIMname[40];
	char	MSNname[40];
	char	ICQname[40];
	char	email[80];
	char	otherinfo[3000];	
	char	blurb[500];
	char	commentname[40];
	char	location[50];
	int	cookienumber;
	char	password[16];
	char	picfilename[40];
	char 	minipicfilename[40];
	int	slideshow;
	char	slideshowname[40];
	int	slideshowdelay;	// how long to delay between images
	long	lastcheckdate;
	int	theme;		// theme #
	int	defaultcam;
	int	defaultbrowser;

	int	verified;
/*  Future features
	int	public;		// Is this record public?
	long	creationdate;
	int	rankingbonus;	// number of hours added to date for ranking
*/
	};


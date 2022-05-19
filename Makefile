
utils.o: utils.c
	gcc -c -g utils.c

lcarsbutton.o: lcarsbutton.c
	gcc -c -g lcarsbutton.c

lcarstab.o: lcarstab.c
	gcc -c -g lcarstab.c

gdtest.o: gdtest.c
	gcc -c -g gdtest.c

listenlive.o: listenlive.c
	gcc -c -g listenlive.c

editsched.o: editsched.c
	gcc -c -g editsched.c

commentlink.o: commentlink.c
	gcc -c -g commentlink.c

vote.o: vote.c
	gcc -c -g vote.c

dispcaps.o: dispcaps.c
	gcc -c -g dispcaps.c

honcap.o: honcap.c
	gcc -c -g honcap.c

lightserver.o: lightserver.c
	gcc -O -c -g lightserver.c

unique.o: unique.c
	gcc -O -c -g unique.c

winamprequest.o: winamprequest.c
	gcc -O -c -g winamprequest.c

icecasthosts.o: icecasthosts.c
	gcc -O -c -g icecasthosts.c

primary1.o: primary1.c
	gcc -c -g primary1.c

visitoredit.o: visitoredit.c
	gcc -c -g visitoredit.c

cookieserver.o: cookieserver.c
	gcc -c -g cookieserver.c

commentserver.o: commentserver.c
	gcc -c -g commentserver.c

cookiecheck.o: cookiecheck.c
	gcc -c -g cookiecheck.c

barcodehtml.o: barcodehtml.c
	gcc -c -g barcodehtml.c

printresponse.o: printresponse.c
	gcc -c -g printresponse.c

pr.o: pr.c
	gcc -c -g pr.c

link.o: link.c
	gcc -c -g link.c

addcam.o: addcam.c
	gcc -c -g addcam.c


br_cmd.o: br_cmd.c
	gcc -c -g br_cmd.c

x10.o: x10.c
	gcc -c -g x10.c

error.o: error.c
	gcc -c -g error.c

read.o: read.c
	gcc -c -g read.c

googlesnacks.o: googlesnacks.c
	gcc -c -g googlesnacks.c

link: link.o utils.o 
	gcc -g  link.o utils.o -lgd -lpng -lz -ljpeg -lm -o link

lcarsbutton: lcarsbutton.o utils.o 
	gcc -g  lcarsbutton.o utils.o -lgd -lpng -lz -ljpeg -lm -lfreetype -o lcarsbutton

lcarstab: lcarstab.o utils.o 
	gcc -g  lcarstab.o utils.o -lgd -lpng -lz -ljpeg -lm -lfreetype -o lcarstab

printresponse: printresponse.o utils.o pr.o
	gcc -g  printresponse.o pr.o utils.o -o printresponse

primary1: primary1.o utils.o
	gcc -g  primary1.o utils.o -o primary1

visitoredit: visitoredit.o utils.o
	gcc -g  visitoredit.o utils.o -o visitoredit

datcreate: datcreate.o utils.o
	gcc -g  datcreate.o utils.o -o datcreate

dathtml: dathtml.o utils.o pr.o
	gcc -g  dathtml.o utils.o pr.o -o dathtml

faqcreate: faqcreate.o utils.o
	gcc -g  faqcreate.o utils.o -o faqcreate

datread: datread.o utils.o
	gcc -g  datread.o utils.o -o datread

disparchive: disparchive.o utils.o
	gcc -g  disparchive.o utils.o -o disparchive

dispcache: dispcache.o utils.o
	gcc -g  dispcache.o utils.o -o dispcache

generate_docam: generate_docam.o utils.o
	gcc -g  generate_docam.o utils.o -o generate_docam

datactivate: datactivate.o utils.o
	gcc -g  datactivate.o utils.o -o datactivate

gettoken: gettoken.o utils.o
	gcc -g  gettoken.o utils.o -o gettoken

gettoken2: gettoken2.o utils.o
	gcc -g  gettoken2.o utils.o -o gettoken2

chattoken: chattoken.o utils.o
	gcc -g  chattoken.o utils.o -o chattoken

test1: test1.o utils.o
	gcc -g  test1.o utils.o -o test1

getstate: getstate.o utils.o
	gcc -g  getstate.o utils.o -o getstate

getinsanity: getinsanity.o utils.o
	gcc -g  getinsanity.o utils.o -o getinsanity

lightserver: lightserver.o utils.o br_cmd.o read.o x10.o
	gcc -O lightserver.o utils.o br_cmd.o read.o x10.o -o lightserver

lightclient: lightclient.o utils.o
	gcc -O lightclient.o utils.o -o lightclient

newlightclient2: newlightclient2.o utils.o
	gcc -O newlightclient2.o utils.o -o newlightclient2


comments: comments.o utils.o pr.o
	gcc -O comments.o pr.o utils.o -o comments

commentlink: commentlink.o utils.o 
	gcc -O commentlink.o utils.o -o commentlink

cookiecheck: cookiecheck.o utils.o
	gcc -O cookiecheck.o utils.o -o cookiecheck

getcamcount: getcamcount.o utils.o
	gcc -O getcamcount.o utils.o -o getcamcount

googlesnacks: googlesnacks.o utils.o
	gcc -O googlesnacks.o utils.o -o googlesnacks

addcam: addcam.o utils.o
	gcc -O addcam.o utils.o -o addcam

barcodehtml: barcodehtml.o utils.o pr.o
	gcc -O barcodehtml.o pr.o utils.o -o barcodehtml

winamprequest: winamprequest.o utils.o
	gcc -O winamprequest.o utils.o -o winamprequest

unique: unique.o utils.o
	gcc -O unique.o utils.o -o unique

icecasthosts: icecasthosts.o utils.o
	gcc -O icecasthosts.o utils.o -o icecasthosts

editsched: editsched.o utils.o
	gcc -O editsched.o utils.o -o editsched

cookieserver: cookieserver.o utils.o
	gcc -O cookieserver.o utils.o -o cookieserver

commentserver: commentserver.o utils.o
	gcc -O commentserver.o utils.o -o commentserver

listcamhosts: listcamhosts.o utils.o
	gcc -O listcamhosts.o utils.o -o listcamhosts

vote: vote.o pr.o utils.o
	gcc -O vote.o pr.o utils.o -o vote

honcap: honcap.o pr.o utils.o
	gcc -O honcap.o pr.o utils.o -o honcap

dispcaps: dispcaps.o pr.o utils.o
	gcc -O dispcaps.o pr.o utils.o -o dispcaps

listenlive: listenlive.o utils.o
	gcc -O listenlive.o utils.o -o listenlive

gdtest: gdtest.o utils.o
	gcc -O gdtest.o utils.o -lgd -lpng -lz -ljpeg -lm -o gdtest



clean:
	rm *.o
	rm listenlive
	rm dispcaps
	rm honcap
	rm vote
	rm listcamhosts
	rm printresponse
	rm primary1
	rm visitoredit
	rm datcreate
	rm dathtml
	rm faqcreate
	rm datread
	rm disparchive
	rm dispcache
	rm generate_docam
	rm commentserver
	rm cookieserver
	rm editsched
	rm icecasthosts
	rm winamprequest
	rm unique
	rm googlesnacks	
	rm barcodehtml
	rm addcam
	rm getcamcount
	rm lightclient
	rm newlightclient2
	rm cookiecheck
	rm commentlink
	rm comments
	rm lightserver
	rm getinsanity
	rm getstate
	rm gettoken
	rm gettoken2
	rm chattoken
	rm deactivate
	rm link
	rm gdtest
	rm lcarsbutton

all:
	make listenlive
	make dispcaps
#	make honcap
	make vote
	make listcamhosts
	make printresponse
	make primary1
	make visitoredit
	make datcreate
	make dathtml
	make faqcreate
	make datread
	make disparchive
	make dispcache
	make generate_docam
	make commentserver
	make cookieserver
	#make editsched
	make icecasthosts
	make winamprequest
	make unique
	make googlesnacks	
	make barcodehtml
#	make addcam
	make getcamcount
	make lightclient
	make newlightclient2
	make cookiecheck
	make comments
	make commentlink
	make lightserver
	make getinsanity
	make getstate
	make gettoken
	make gettoken2
	make chattoken
	make link
	make gdtest
	make lcarsbutton
	#make deactivate

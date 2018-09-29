#	spline make file
#	@(#)spline.mk	1.4
#	NOTE: This makefile is only necessary because of the need for the
#		base register option for cc on u370.

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid spline $(REL)`
MKSID = -r`gsid spline.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -n
B02 = `if u370; then echo "-b0,2"; fi`
CFLAGS = -O $(B02)
LDFLAGS = -s $(IFLAG) $(FFLAG)
SOURCE = spline.c
MAKE = make

compile all: spline
	:

spline:
	$(CC) $(CFLAGS) $(LDFLAGS) -o spline spline.c
	$(INS) $(INSDIR) spline

install:
	$(MAKE) -f spline.mk INS="install -f" OL=$(OL)

build:	bldmk
	get -p $(CSID) s.spline.c $(REWIRE) > $(RDIR)/spline.c
bldmk:  ;  get -p $(MKSID) s.spline.mk > $(RDIR)/spline.mk

listing:
	pr spline.mk $(SOURCE) | $(LIST)
listmk: ;  pr spline.mk | $(LIST)

edit:
	get -e s.spline.c

delta:
	delta s.spline.c

mkedit:  ;  get -e s.spline.mk
mkdelta: ;  delta s.spline.mk

clean:
	:

clobber:
	  rm -f spline

delete:	clobber
	rm -f $(SOURCE)

#	diff make file
#	SCCS:	@(#)diff.mk	1.6

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)/diff
INS = :
REL = current
CSID = -r`gsid diff $(REL)`
MKSID = -r`gsid diff.mk $(REL)`
LIST = lp
INSDIR = $(OL)bin
INSLIB = $(OL)usr/lib
IFLAG = -i
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SOURCE = diff.c diffh.c
MAKE = make

VPATH: all ;	ROOT=`pwd`
compile all: diff diffh
	:

diff:
	$(CC) $(CFLAGS) $(LDFLAGS) -o diff diff.c
	$(INS) $(INSDIR) diff 

diffh:
	$(CC) $(CFLAGS) $(LDFLAGS) -o diffh diffh.c
	$(INS) $(INSLIB) diffh

install:
	$(MAKE) -f diff.mk INS="install -f" OL=$(OL) IFLAG=$(IFLAG)

build:	bldmk
	get -p $(CSID) s.diff.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:  ;  get -p $(MKSID) s.diff.mk > $(RDIR)/diff.mk

listing:
	pr diff.mk $(SOURCE) | $(LIST)
listmk: ;  pr diff.mk | $(LIST)

edit:
	get -e -p s.diff.src | ntar -g

delta:
	ntar -p $(SOURCE) > diff.src
	delta s.diff.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.diff.mk
mkdelta: ;  delta s.diff.mk

clean:
	:

clobber:
	  rm -f diff diffh

delete:	clobber
	rm -f $(SOURCE)

#	dc make file
#	SCCS:	@(#)dc.mk	1.7

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)/dc
INS = :
REL = current
CSID = -r`gsid dc $(REL)`
MKSID = -r`gsid dc.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -i
B30 = `if u370; then echo "-b3,0"; fi`
LDFLAGS = -s $(IFLAG)
CFLAGS = `if u370; then echo -n -O; else echo -O; fi`
SOURCE = dc.h dc.c
MAKE = make

compile all: dc
	:

dc:	dc.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o dc dc.c
	$(INS) $(INSDIR) dc


install:
	$(MAKE) -f dc.mk INS="install -f" OL=$(OL) IFLAG=$(IFLAG)

build:	bldmk
	get -p $(CSID) s.dc.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:  ;  get -p $(MKSID) s.dc.mk > $(RDIR)/dc.mk

listing:
	pr dc.mk $(SOURCE) | $(LIST)
listmk: ;  pr dc.mk | $(LIST)

edit:
	get -e -p s.dc.src | ntar -g

delta:
	ntar -p $(SOURCE) > dc.src
	delta s.dc.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.dc.mk
mkdelta: ;  delta s.dc.mk

clean:
	:

clobber:
	  rm -f dc

delete:	clobber
	rm -f $(SOURCE)

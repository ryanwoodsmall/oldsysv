#	unpack (pcat) make file
#	SCCS:	@(#)unpack.mk	1.6

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid unpack $(REL)`
MKSID = -r`gsid unpack.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SOURCE = unpack.c
MAKE = make

compile all: unpack
	:

unpack:	
	$(CC) $(CFLAGS) $(LDFLAGS) -o unpack unpack.c
	$(INS) $(INSDIR) unpack
	rm -f $(INSDIR)/pcat
	ln $(INSDIR)/unpack $(INSDIR)/pcat

pcat:	unpack
	:

install:
	$(MAKE) -f unpack.mk INS="install -f" OL=$(OL)

build:	bldmk
	get -p $(CSID) s.unpack.c $(REWIRE) > $(RDIR)/unpack.c
bldmk:
	get -p $(MKSID) s.unpack.mk > $(RDIR)/unpack.mk
	rm -f $(RDIR)/pcat.mk
	ln $(RDIR)/unpack.mk $(RDIR)/pcat.mk

listing:
	pr unpack.mk $(SOURCE) | $(LIST)
listmk: ;  pr unpack.mk | $(LIST)

edit:
	get -e s.unpack.c

delta:
	delta s.unpack.c

mkedit:  ;  get -e s.unpack.mk
mkdelta: ;  delta s.unpack.mk

clean:
	:

clobber:
	  rm -f unpack

delete:	clobber
	rm -f $(SOURCE)

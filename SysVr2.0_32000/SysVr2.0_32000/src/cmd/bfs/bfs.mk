#	bfs make file
#	SCCS:	@(#)bfs.mk	1.6

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -n
LDFLAGS = -s $(IFLAG) $(FFLAG)
CFLAGS = -O
SOURCE = bfs.c
MAKE = make

compile all: bfs
	:

bfs:
	$(CC) $(CFLAGS) $(LDFLAGS) -o bfs bfs.c
	$(INS) $(INSDIR) bfs

install:
	$(MAKE) -f bfs.mk INS="install -f" OL=$(OL)

build:	bldmk
	get -p -r`gsid bfs $(REL)` s.bfs.c $(REWIRE) > $(RDIR)/bfs.c
bldmk:  ;  get -p -r`gsid bfs.mk $(REL)` s.bfs.mk > $(RDIR)/bfs.mk

listing:
	pr bfs.mk $(SOURCE) | $(LIST)
listmk: ;  pr bfs.mk | $(LIST)

edit:
	get -e s.bfs.c

delta:
	delta s.bfs.c
	rm -f $(SOURCE)

mkedit:  ;  get -e s.bfs.mk
mkdelta: ;  delta s.bfs.mk

clean:
	  :

clobber:  clean
	  rm -f bfs

delete:	clobber
	rm -f $(SOURCE)

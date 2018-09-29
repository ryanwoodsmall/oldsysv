#	ed make file
#	SCCS:	@(#)ed.mk	1.9

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)/ed
INS = :
REL = current
CSID = -r`gsid ed $(REL)`
FUNSID = -r`gsid edfun $(REL)`
MKSID = -r`gsid ed.mk $(REL)`
LIST = lp
INSDIR = $(OL)bin
IFLAG = -n
B10 = 
CFLAGS = -O $(B10)
LDFLAGS = -s $(IFLAG)
FIXSOURCE = edfun
SOURCE = ed.c
MAKE = make

compile all: ed
	:

ed:	ed.s
	$(CC) $(LDFLAGS) -o ed ed.s
	$(INS) $(INSDIR) ed
	rm -f $(INSDIR)/red
	ln $(INSDIR)/ed $(INSDIR)/red

ed.s:	ed.c
	$(CC) -S $(CFLAGS) ed.c
	-if pdp11; then sh ./edfun ed.s; else true; fi

install:
	$(MAKE) -f ed.mk INS="install -f" OL=$(OL)

build:	bldmk bldedfun
	get -p $(CSID) s.ed.c $(REWIRE) > $(RDIR)/ed.c
bldedfun:
	get -p $(FUNSID) s.edfun $(REWIRE) > $(RDIR)/edfun
bldmk:  ;  get -p $(MKSID) s.ed.mk > $(RDIR)/ed.mk

listing:
	pr ed.mk $(SOURCE) | $(LIST)
listmk: ;  pr ed.mk | $(LIST)

edit:	; get -e s.ed.c
edfunedit:	; get -e s.edfun

delta:	; delta s.ed.c
edfundelta:	; delta s.edfun

mkedit:  ;  get -e s.ed.mk
mkdelta: ;  delta s.ed.mk

clean:
	  rm -f ed.s

clobber:  clean
	  rm -f ed

delete:	clobber
	rm -f $(SOURCE) $(FIXSOURCE)

#	calendar make file
#	SCCS:	@(#)calendar.mk	1.6

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)/calendar
INS = :
REL = current
LIST = lp
PINSDIR = $(OL)usr/lib
INSDIR = $(OL)usr/bin
IFLAG = -n
LDFLAGS = -s $(IFLAG)
CFLAGS = -O
SHSOURCE = calendar.sh
PSOURCE = calprog.c
SHFILES = calendar
MAKE = make

compile all: calendar calprog
	:

calendar:
	cp calendar.sh calendar
	$(INS) $(INSDIR) calendar

calprog:
	$(CC) $(CFLAGS) $(LDLAGS) -o calprog calprog.c
	$(INS) $(PINSDIR) calprog

install:
	$(MAKE) -f calendar.mk INS="install -f" OL=$(OL) $(ARGS)
inscalp:
	$(MAKE) -f calendar.mk INS=cp OL=$(OL) calprog

build:	bldmk bldcalp
	get -p -r`gsid calendar $(REL)` s.calendar.sh $(REWIRE) > $(RDIR)/calendar.sh
bldcalp:
	get -p -r`gsid calprog $(REL)` s.calprog.c $(REWIRE) > $(RDIR)/calprog.c
bldmk:  ;  get -p -r`gsid calendar.mk $(REL)` s.calendar.mk > $(RDIR)/calendar.mk

listing:
	pr calendar.mk $(SHSOURCE) $(PSOURCE) | $(LIST)
lstcal: ; pr $(SHSOURCE) | $(LIST)
lstcalp: ; pr $(PSOURCE) | $(LIST)
listmk: ;  pr calendar.mk | $(LIST)

caledit: ; get -e s.calendar.sh
calpedit: ; get -e s.calprog.c

caldelta: ; delta s.calendar.sh
calpdelta: ; delta s.calprog.c

mkedit:  ;  get -e s.calendar.mk
mkdelta: ;  delta s.calendar.mk

clean: ;   :
calclean: ; :
calpclean: ; :

clobber:
	rm -f calendar calprog
calclobber:
	rm -f calendar
calpclobber:
	rm -f calprog

delete:	clobber
	rm -f $(SHSOURCE) $(PSOURCE)
caldelete:	calclobber
	rm -f $(SHSOURCE)
calpdelete:	calpclobber
	rm -f $(PSOURCE)

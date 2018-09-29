#	deroff make file
#	SCCS:	@(#)deroff.mk	1.1
#	NOTE: This makefile is only necessary because of the need for the
#		base register option for cc on u370.

OL = /
SL = /usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid deroff $(REL)`
MKSID = -r`gsid deroff.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -n
B20 =
CFLAGS = -O $(B20)
LDFLAGS = -s $(IFLAG)
SOURCE = deroff.c
MAKE = make

compile all: deroff
	:

deroff:
	$(CC) $(CFLAGS) $(LDFLAG) -o deroff deroff.c
	$(INS) deroff $(INSDIR)
	chmod 775 $(INSDIR)/deroff
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin deroff; chgrp bin deroff; \
	 fi

install:
	$(MAKE) -f deroff.mk INS=cp OL=$(OL)

build:	bldmk
	get -p $(CSID) s.deroff.c $(REWIRE) > $(RDIR)/deroff.c
bldmk:  ;  get -p $(MKSID) s.deroff.mk > $(RDIR)/deroff.mk

listing:
	pr deroff.mk $(SOURCE) | $(LIST)
listmk: ;  pr deroff.mk | $(LIST)

edit:
	get -e s.deroff.c

delta:
	delta s.deroff.c

mkedit:  ;  get -e s.deroff.mk
mkdelta: ;  delta s.deroff.mk

clean:
	:

clobber:  clean
	  rm -f deroff

delete:	clobber
	rm -f $(SOURCE)

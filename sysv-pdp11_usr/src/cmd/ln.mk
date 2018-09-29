#	mv/cp/ln make file
#	@(#)ln.mk	1.1

OL = /
SL = /usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid mv $(REL)`
MKSID = -r`gsid mv.mk $(REL)`
LIST = lp
INSDIR = $(OL)bin
TESTDIR = .
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SOURCE = mv.c
MAKE = make

compile all: mv
	:

mv:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/mv mv.c
	$(INS) $(TESTDIR)/mv $(INSDIR); \
		cp $(TESTDIR)/mv $(TESTDIR)/ln; \
		chmod 755 $(TESTDIR)/ln; \
		rm -f $(INSDIR)/ln; \
		cp $(TESTDIR)/ln $(INSDIR); \
		$(TESTDIR)/ln $(INSDIR)/ln $(INSDIR)/cp
	chmod 775 $(INSDIR)/mv $(INSDIR)/ln
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin mv ln; chgrp bin mv ln; \
	 fi
	-if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown root mv; chmod 4775 mv; \
	 fi

install:
	$(MAKE) -f mv.mk INS=cp OL=$(OL)

build:	bldmk
	get -p $(CSID) s.mv.c $(REWIRE) > $(RDIR)/mv.c
bldmk:  ;  get -p $(MKSID) s.mv.mk > $(RDIR)/mv.mk
	cd $(RDIR); rm -f ln.mk cp.mk
	cd $(RDIR); ln mv.mk cp.mk; ln mv.mk ln.mk

listing:
	pr mv.mk $(SOURCE) | $(LIST)
listmk: ;  pr mv.mk | $(LIST)

edit:
	get -e s.mv.c

delta:
	delta s.mv.c
	rm -f $(SOURCE)

mkedit:  ;  get -e s.mv.mk
mkdelta: ;  delta s.mv.mk

clean:
	:

clobber:
	rm -f $(TESTDIR)/mv $(TESTDIR)/ln

delete:	clobber
	rm -f $(SOURCE)

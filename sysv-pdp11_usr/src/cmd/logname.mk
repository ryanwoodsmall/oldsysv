#	logname make file
#	SCCS:	@(#)logname.mk	1.4

OL = /
SL = /usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid logname $(REL)`
MKSID = -r`gsid logname.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SOURCE = logname.c
MAKE = make

compile all: logname
	:

logname:
	$(CC) $(CFLAGS) $(LDFLAGS) -o logname logname.c -lPW
	$(INS) logname $(INSDIR)
	chmod 775 $(INSDIR)/logname
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin logname; chgrp bin logname; \
	 fi

install:
	$(MAKE) -f logname.mk INS=cp OL=$(OL)

build:	bldmk
	get -p $(CSID) s.logname.c $(REWIRE) > $(RDIR)/logname.c
bldmk:  ;  get -p $(MKSID) s.logname.mk > $(RDIR)/logname.mk

listing:
	pr logname.mk $(SOURCE) | $(LIST)
listmk: ;  pr logname.mk | $(LIST)

edit:
	get -e s.logname.c

delta:
	delta s.logname.c

mkedit:  ;  get -e s.logname.mk
mkdelta: ;  delta s.logname.mk

clean:
	:

clobber:
	  rm -f logname

delete:	clobber
	rm -f $(SOURCE)

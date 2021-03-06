#	regcmp make file
#	SCCS:	@(#)regcmp.mk	1.3

OL = /
SL = /usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid regcmp $(REL)`
MKSID = -r`gsid regcmp.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SOURCE = regcmp.c
MAKE = make

compile all: regcmp
	:

regcmp:
	$(CC) $(CFLAGS) $(LDFLAGS) -o regcmp regcmp.c -lPW
	$(INS) regcmp $(INSDIR)
	chmod 775 $(INSDIR)/regcmp
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin regcmp; chgrp bin regcmp; \
	 fi

install:
	$(MAKE) -f regcmp.mk INS=cp OL=$(OL)

build:	bldmk
	get -p $(CSID) s.regcmp.c $(REWIRE) > $(RDIR)/regcmp.c
bldmk:  ;  get -p $(MKSID) s.regcmp.mk > $(RDIR)/regcmp.mk

listing:
	pr regcmp.mk $(SOURCE) | $(LIST)
listmk: ;  pr regcmp.mk | $(LIST)

edit:
	get -e s.regcmp.c

delta:
	delta s.regcmp.c

mkedit:  ;  get -e s.regcmp.mk
mkdelta: ;  delta s.regcmp.mk

clean:
	:

clobber:
	  rm -f regcmp

delete:	clobber
	rm -f $(SOURCE)

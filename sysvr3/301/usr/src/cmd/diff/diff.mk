#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)diff:diff.mk	1.8"
#	diff make file

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
LDFLAGS = -s $(IFLAG) $(LDLIBS)
SOURCE = diff.c diffh.c
MAKE = make

VPATH: all ;	ROOT=`pwd`
compile all: diff diffh
	:

diff:
	$(CC) $(CFLAGS) -o diff diff.c $(LDFLAGS)
	$(INS) $(INSDIR) diff 

diffh:
	$(CC) $(CFLAGS) -o diffh diffh.c $(LDFLAGS)
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

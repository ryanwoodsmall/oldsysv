#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fmthard:fmthard.mk	1.4"

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install
FRC =

all:	fmthard

install: all
	$(INS) -n $(ROOT)/etc fmthard

fmthard:
	$(CC) $(CFLAGS) $(LDFLAGS) -o fmthard fmthard.c $(LDLIBS)

clean:
	-rm -f fmthard.o

clobber: clean
	-rm -f fmthard

FRC:

#
# Header dependencies
#

fmthard: fmthard.c \
	$(INCRT)/fcntl.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/id.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)

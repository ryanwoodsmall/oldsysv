#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fmtflop:fmtflop.mk	1.3"
#
#		Copyright 1984 AT&T
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install

all:	fmtflop

install: all
	$(INS) -n $(ROOT)/etc fmtflop

fmtflop:
	$(CC) $(CFLAGS) $(LDFLAGS) -o fmtflop fmtflop.c $(LDLIBS)

clean:
	-rm -f fmtflop.o

clobber: clean
	-rm -f fmtflop

FRC:

#
# Header dependencies
#

fmtflop: fmtflop.c \
	$(INCRT)/errno.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/if.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)

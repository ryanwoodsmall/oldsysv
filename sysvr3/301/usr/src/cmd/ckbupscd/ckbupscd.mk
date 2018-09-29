#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ckbupscd:ckbupscd.mk	1.2"
#
#		Copyright 1984 AT&T
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
INS = install
FRC =

all:	 ckbupscd

install: all
	$(INS) -f $(ROOT)/etc ckbupscd

ckbupscd:
	$(CC) $(CFLAGS) $(LDFLAGS) -o ckbupscd ckbupscd.c $(LDLIBS)

clean:
	-rm -f ckbupscd.o

clobber: clean
	-rm -f ckbupscd

FRC:

#
# Header dependencies
#

ckbupscd: ckbupscd.c \
	$(INCRT)/time.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/types.h \
	$(FRC)

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mcs:mcs.mk	1.2"
ROOT=
FRC =
INSDIR = $(ROOT)/usr/bin 
LDFLAGS = -s

all:	mcs

mcs:	mcs.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mcs mcs.c

install:	mcs
	cpset mcs $(INSDIR)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f mcs

FRC:


#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident "@(#)idload:idload.mk	1.3"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
LDFLAGS = -lns $(LDLIBS)
FRC =

all: idload

idload: idload.c
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/idload idload.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/idload

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/idload
FRC:

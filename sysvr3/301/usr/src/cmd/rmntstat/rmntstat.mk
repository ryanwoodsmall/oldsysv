#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rmntstat:rmntstat.mk	1.1"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
FRC =

all: rmntstat

rmntstat: rmntstat.o getnodes.o
	$(CC) -o $(TESTDIR)/rmntstat rmntstat.o getnodes.o

rmntstat.o : rmntstat.c fumount.h
	$(CC) -I$(INCRT) -c $(CFLAGS) rmntstat.c

getnodes.o : getnodes.c fumount.h
	$(CC) -I$(INCRT) -c $(CFLAGS) getnodes.c

install: all
	install -n $(INSDIR) $(TESTDIR)/rmntstat

clean:
	rm -f rmntstat.o
	rm -f getnodes.o

clobber: clean
	rm -f $(TESTDIR)/rmntstat
FRC:

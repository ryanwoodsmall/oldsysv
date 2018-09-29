#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nsquery:nsquery.mk	1.5"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
LDFLAGS = -lns $(LDLIBS)
INS = :
CFLAGS = -O -s
FRC =

all: nsquery

nsquery: nsquery.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/nsquery nsquery.c $(LDFLAGS)

install: all
	cpset nsquery $(INSDIR) 04555 root bin

clean:
	rm -f nsquery.o

clobber: clean
	rm -f $(TESTDIR)/nsquery
FRC:

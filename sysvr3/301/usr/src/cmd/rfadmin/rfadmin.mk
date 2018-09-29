#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfadmin:rfadmin.mk	1.1"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
LDFLAGS = -lns -lcrypt $(LDLIBS)
FRC =

all: rfadmin

rfadmin: rfadmin.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/rfadmin rfadmin.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/rfadmin

clean:
	rm -f rfadmin.o

clobber: clean
	rm -f $(TESTDIR)/rfadmin
FRC:

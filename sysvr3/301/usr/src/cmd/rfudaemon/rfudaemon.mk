#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfudaemon:rfudaemon.mk	1.1"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
FRC =

all: rfudaemon

rfudaemon: rfudaemon.c
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/rfudaemon rfudaemon.c

install: all
	install -n $(INSDIR) $(TESTDIR)/rfudaemon

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/rfudaemon
FRC:

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dustart:dustart.mk	1.6"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
LDFLAGS = -lns $(LDLIBS)
FRC =

all: rfstart

rfstart: rfstart.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/rfstart rfstart.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/rfstart

clean:
	rm -f rfstart.o

clobber: clean
	rm -f $(TESTDIR)/rfstart
FRC:

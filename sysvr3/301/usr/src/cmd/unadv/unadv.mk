#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)unadv:unadv.mk	1.5"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
LDFLAGS = -lns $(LDLIBS)
INS = :
CFLAGS = -O -s
FRC =

all: unadv

unadv: unadv.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/unadv unadv.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/unadv

clean:
	rm -f unadv.o

clobber: clean
	rm -f $(TESTDIR)/unadv
FRC:

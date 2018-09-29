#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dname:dname.mk	1.3"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
LDFLAGS = -lns $(LDLIBS)
FRC =

all: dname

dname: dname.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/dname dname.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/dname

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/dname
FRC:

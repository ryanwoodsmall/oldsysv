#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fsstat:fsstat.mk	1.3"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
LDFLAGS = -s $(LDLIBS)
FRC =

all: fsstat

fsstat: fsstat.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/fsstat fsstat.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/fsstat

clean:

clobber: clean
	rm -f $(TESTDIR)/fsstat
FRC:

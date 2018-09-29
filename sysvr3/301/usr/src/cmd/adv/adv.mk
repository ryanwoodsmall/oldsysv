#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)adv:adv.mk	1.4"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INCRT = $(ROOT)/usr/include
LDFLAGS = -lns
INS = :
CFLAGS = -O -s
FRC =

all: adv

adv: adv.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/adv adv.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/adv

clean:
	rm -f adv.o

clobber: clean
	rm -f $(TESTDIR)/adv
FRC:

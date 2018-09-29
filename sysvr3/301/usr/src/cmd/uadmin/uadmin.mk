#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)uadmin:uadmin.mk	1.4"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O
LDFLAGS = -s 
FRC =

all: uadmin

uadmin: uadmin.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/uadmin uadmin.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/uadmin

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/uadmin
FRC:

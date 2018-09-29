#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)getty:getty.mk	1.5"
TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/etc 
LDFLAGS = -s -n

all: getty

getty: getty.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/getty getty.c  $(LDLIBS)

test:
	rtest $(TESTDIR)/getty

install: all
	$(INS) -f $(INSDIR) -o $(TESTDIR)/getty $(INSDIR)

clean:


clobber: clean
	-rm -f $(TESTDIR)/getty

FRC:

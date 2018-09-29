#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)init:init.mk	1.7"
TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/etc 
LDFLAGS = -O -s

all: init

init: init.c $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/init init.c  

test:
	rtest $(TESTDIR)/init

install: all
	$(INS) -f $(INSDIR) -o $(TESTDIR)/init $(INSDIR)
	-rm -f $(ROOT)/bin/telinit
	ln $(ROOT)/etc/init $(ROOT)/bin/telinit

clean:


clobber: clean
	-rm -f $(TESTDIR)/init

FRC:

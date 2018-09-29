#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)file:file.mk	1.6"
#	@(#)makefile	1.1
#
#	Makefile for file command -- necessary to install /etc/magic
#
ROOT =
TESTDIR=.
INS = install
INSDIR = $(ROOT)/bin
MINSDIR = $(ROOT)/etc
LDFLAGS = -s

all:	file

file:	file.c
	$(CC) $(CFLAGS) $(LDFLAGS) file.c -o $(TESTDIR)/file $(LDLIBS)

install: all
	$(INS) -n $(INSDIR) $(TESTDIR)/file
	$(INS) -n $(MINSDIR) magic

clean:
	-rm -f file.o

clobber: clean
	-rm -f $(TESTDIR)/file

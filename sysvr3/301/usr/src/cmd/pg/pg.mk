#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pg:pg.mk	1.5"
ROOT =
TESTDIR = .
INS = install
LIBCURSES=-lcurses
LDFLAGS = -O -s -i

all: pg

pg: pg.c
	$(CC) -DSINGLE $(LDFLAGS) -o $(TESTDIR)/pg pg.c $(LIBCURSES)

install: all
	$(INS) -n $(ROOT)/usr/bin $(TESTDIR)/pg

clean:
	-rm -f pg.o

clobber: clean
	-rm -f $(TESTDIR)/pg

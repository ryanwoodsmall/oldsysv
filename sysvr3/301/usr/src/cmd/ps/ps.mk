#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ps:ps.mk	1.7"
#	ps make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s -lgen $(LDLIBS)
INS=:

ps:
	$(CC) -o ps ps.c $(LDFLAGS)
	$(INS) $(INSDIR) -m 2755 -u root -g sys ps

all:	install clobber

install:
	$(MAKE) -f ps.mk INS="install -f"

clean:
	-rm -f ps.o

clobber: clean
	rm -f ps

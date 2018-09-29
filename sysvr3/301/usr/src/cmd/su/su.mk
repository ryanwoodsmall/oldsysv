#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)su:su.mk	1.9"
#	su make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s -lcrypt 
INS=:

su:
	$(CC) -o su su.c $(LDFLAGS)
	$(INS) $(INSDIR) -m 4755 -u root -g sys su

all:	install clobber

install:
	$(MAKE) -f su.mk INS="install -f"

clean:


clobber: clean
	rm -f su

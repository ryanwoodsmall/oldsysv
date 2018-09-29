#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rmdir:rmdir.mk	1.9"
#	rmdir make file
ROOT=
INSDIR = $(ROOT)/bin
CFLAGS = -O 
LDFLAGS = -s -lgen $(LDLIBS)
INS=:

rmdir:
	$(CC) $(CFLAGS) -o rmdir rmdir.c $(LDFLAGS)
	$(INS) $(INSDIR) rmdir

all:	install clobber

install:
	$(MAKE) -f rmdir.mk INS="install -f"

clean:
	-rm -f rmdir.o

clobber: clean
	rm -f rmdir

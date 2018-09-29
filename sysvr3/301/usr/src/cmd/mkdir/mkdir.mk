#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	@(#)mkdir:mkdir.mk	1.11
#	mkdir make file
ROOT=
INSDIR = $(ROOT)/bin
CFLAGS = -O
LDFLAGS =  -s -lgen $(LDLIBS)
INS=:

mkdir:
	$(CC) $(CFLAGS) -o mkdir mkdir.c  $(LDFLAGS)
	$(INS) $(INSDIR) mkdir

all:	install clobber

install:
	$(MAKE) -f mkdir.mk INS="install -f"

clean:
	-rm -f mkdir.o

clobber: clean
	rm -f mkdir

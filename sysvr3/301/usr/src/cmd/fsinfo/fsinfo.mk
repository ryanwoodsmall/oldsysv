#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident "@(#)fsinfo:fsinfo.mk	1.1"
#
#		Copyright 1984 AT&T
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT)
LDFLAGS = -s
INS = install

all:	fsinfo

install: all
	$(INS) -f $(ROOT)/usr/lbin fsinfo

fsinfo:
	$(CC) $(CFLAGS) $(LDFLAGS) -o fsinfo fsinfo.c

clean:
	-rm -f fsinfo.o

clobber: clean
	-rm -f fsinfo

FRC:

#
# Header dependencies
#

fsinfo: fsinfo.c \
	$(INCRT)/fcntl.h \
	$(INCRT)/sys/filsys.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(FRC)

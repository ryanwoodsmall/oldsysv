#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)swap:swap.mk	1.4"
#
#		Copyright 1985 AT&T
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT)
LDFLAGS = -s
INS = install

all:	swap

install: all
	$(INS) -f $(ROOT)/etc swap

swap:
	$(CC) $(CFLAGS) $(LDFLAGS) -o swap swap.c $(LDLIBS)

clean:
	-rm -f swap.o

clobber: clean
	-rm -f swap

FRC:

#
# Header dependencies
#

swap:	swap.c \
	$(INCRT)/stdio.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sys3b.h \
	$(FRC)

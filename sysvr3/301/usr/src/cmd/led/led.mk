#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)led:led.mk	1.3"
#
#		Copyright 1984 AT&T
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install
FRC =

all:	led

led:
	$(CC) $(CFLAGS) $(LDFLAGS) -o led led.c $(LDLIBS)

install: all
	$(INS) -n $(ROOT)/etc led

clean:
	-rm -f led.o

clobber: clean
	-rm -f led

FRC:

#
# Header dependencies
#

led: led.c \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/sys3b.h \
	$(FRC)

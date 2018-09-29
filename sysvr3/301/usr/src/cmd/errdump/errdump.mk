#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)errdump:errdump.mk	1.4"
#
#		Copyright 1984 AT&T
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install

all:	errdump

errdump:
	$(CC) $(CFLAGS) $(LDFLAGS) -o errdump errdump.c $(LDLIBS)

install: all
	$(INS) -n $(ROOT)/etc errdump

clean:
	-rm -f errdump.o

clobber: clean
	-rm -f errdump

FRC:

#
# Header dependencies
#

errdump: errdump.c \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/time.h \
	$(FRC)

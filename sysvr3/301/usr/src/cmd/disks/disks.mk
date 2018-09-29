#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)disks:disks.mk	1.7"

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
INS = install
FRC =

all:	 disks

install: all
	$(INS) -f $(ROOT)/etc -m 755 -u root -g sys disks

disks:
	$(CC) $(CFLAGS) $(LDFLAGS) -o disks disks.c 

clean:
	-rm -f disks.o

clobber: clean
	-rm -f disks

FRC:

#
# Header dependencies
#

disks: disks.c \
	$(INCRT)/sys/edt.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(FRC)

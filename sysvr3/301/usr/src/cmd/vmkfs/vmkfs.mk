#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vmkfs:vmkfs.mk	1.2"

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
INS = install
FRC =

all:	 vmkfs

install: all
	$(INS) -f $(ROOT)/usr/lbin vmkfs

vmkfs:
	$(CC) $(CFLAGS) $(LDFLAGS) -o vmkfs vmkfs.c

clean:
	-rm -f vmkfs.o

clobber: clean
	-rm -f vmkfs

FRC:

#
# Header dependencies
#

vmkfs: vmkfs.c \
	$(INCRT)/errno.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/id.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)

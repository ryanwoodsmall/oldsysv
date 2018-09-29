#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)devinfo:devinfo.mk	1.3"
ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install
FRC =

all:	devinfo

devinfo:
	$(CC) $(CFLAGS) $(LDFLAGS) -o devinfo devinfo.c $(LDLIBS)

install: all
	$(INS) -n $(ROOT)/usr/lbin devinfo

clean:
	-rm -f devinfo.o

clobber: clean
	-rm -f devinfo

FRC:

#
# Header dependencies
#

devinfo: devinfo.c \
	$(INCRT)/fcntl.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/id.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)

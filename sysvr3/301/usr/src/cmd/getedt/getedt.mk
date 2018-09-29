#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)getedt:getedt.mk	1.4"
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

all:	 getedt

install: all
	$(INS) -n $(ROOT)/usr/lbin getedt

getedt:
	$(CC) $(CFLAGS) $(LDFLAGS) -o getedt getedt.c $(LDLIBS)

clean:
	-rm -f getedt.o

clobber: clean
	-rm -f getedt

FRC:

#
# Header dependencies
#

getedt: getedt.c \
	$(INCRT)/fcntl.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/hdeioctl.h \
	$(INCRT)/sys/hdelog.h \
	$(INCRT)/sys/id.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	../bbh/edio.h \
	../bbh/hdecmds.h \
	$(FRC)

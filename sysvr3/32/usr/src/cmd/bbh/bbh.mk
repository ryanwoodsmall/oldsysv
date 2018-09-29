#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bbh:bbh.mk	1.7.1.2"
ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install

all:	hdeadd hdefix hdelogger

hdeadd:	dconv.o edio.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o hdeadd hdeadd.c dconv.o edio.o $(LDLIBS)

hdefix:	edio.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o hdefix hdefix.c edio.o $(LDLIBS)

hdelogger: edio.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o hdelogger hdelogger.c edio.o $(LDLIBS)

install: all
	$(INS) -f $(ROOT)/etc hdeadd
	$(INS) -f $(ROOT)/etc hdefix
	$(INS) -o -f $(ROOT)/etc hdelogger

clean:
	-rm -f hdeadd.o hdefix.o hdelogger.o edio.o dconv.o

clobber: clean
	-rm -f hdeadd hdefix hdelogger

FRC:

#
# Header dependencies
#

hdeadd: hdeadd.c \
	$(INC)/fcntl.h \
	$(INC)/stdio.h \
	$(INC)/sys/hdeioctl.h \
	$(INC)/sys/hdelog.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vtoc.h \
	dconv.h \
	edio.h \
	hdecmds.h \
	$(FRC)

hdefix: hdefix.c \
	$(INC)/fcntl.h \
	$(INC)/stdio.h \
	$(INC)/sys/filsys.h \
	$(INC)/sys/hdeioctl.h \
	$(INC)/sys/hdelog.h \
	$(INC)/sys/param.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	$(INC)/sys/uadmin.h \
	$(INC)/sys/vtoc.h \
	$(INC)/utmp.h \
	edio.h \
	hdecmds.h \
	$(FRC)

hdelogger: hdelogger.c \
	$(INC)/stdio.h \
	$(INC)/sys/hdeioctl.h \
	$(INC)/sys/hdelog.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vtoc.h \
	$(INC)/time.h \
	edio.h \
	hdecmds.h \
	$(FRC)

edio.o: edio.c \
	$(INC)/errno.h \
	$(INC)/fcntl.h \
	$(INC)/stdio.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/hdeioctl.h \
	$(INC)/sys/hdelog.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vtoc.h \
	edio.h \
	hdecmds.h \
	$(FRC)

dconv.o: dconv.c \
	$(INC)/time.h \
	$(FRC)

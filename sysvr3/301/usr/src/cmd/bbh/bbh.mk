#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bbh:bbh.mk	1.7"
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
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
	$(INS) -n $(ROOT)/etc hdeadd
	$(INS) -n $(ROOT)/etc hdefix
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
	$(INCRT)/fcntl.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/hdeioctl.h \
	$(INCRT)/sys/hdelog.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	dconv.h \
	edio.h \
	hdecmds.h \
	$(FRC)

hdefix: hdefix.c \
	$(INCRT)/fcntl.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/filsys.h \
	$(INCRT)/sys/hdeioctl.h \
	$(INCRT)/sys/hdelog.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/uadmin.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/utmp.h \
	edio.h \
	hdecmds.h \
	$(FRC)

hdelogger: hdelogger.c \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/hdeioctl.h \
	$(INCRT)/sys/hdelog.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/time.h \
	edio.h \
	hdecmds.h \
	$(FRC)

edio.o: edio.c \
	$(INCRT)/errno.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/hdeioctl.h \
	$(INCRT)/sys/hdelog.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	edio.h \
	hdecmds.h \
	$(FRC)

dconv.o: dconv.c \
	$(INCRT)/time.h \
	$(FRC)

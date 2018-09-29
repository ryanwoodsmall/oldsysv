#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/xt.mk	10.7"
ROOT =
STRIP= strip
DBO = -DDBO
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INCRT = $(ROOT)/usr/include
NOFLEX = -Wp,-T -W0,-XT
CFLAGS = -I$(INCRT) -Uvax -Uu3b -Uu3b5 -Du3b2 -DINKERNEL -Updp11 $(DBO)
FRC =

DFILES =\
	xt.o


OPFILES = \
	xt

install:	drivers

drivers:	all

all:	$(DFILES)


clean:

clobber:	clean
	-rm -f xt.o

FRC:

#
# Header dependencies
#

xt.o: \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/callo.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/jioctl.h \
	$(INCRT)/sys/xtproto.h \
	$(INCRT)/sys/xt.h \
	$(FRC)
	$(CC) $(CFLAGS) -c xt.c

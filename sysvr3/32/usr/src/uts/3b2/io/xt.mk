#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/xt.mk	10.8.2.3"
ROOT =
STRIP= strip
DBO = -DDBO
MAKE = make
MAKE = $(MAKE) "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INC = $(ROOT)/usr/include
NOFLEX = -Wp,-T -W0,-XT
DASHO = -O
CFLAGS = $(DASHO) -I$(INC) -Uvax -Uu3b -Uu3b15 -Du3b2 -DINKERNEL -Updp11 $(DBO)
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

xt.o: xt.c \
	$(INC)/sys/sccsid.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/file.h \
	$(INC)/sys/ino.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/callo.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/jioctl.h \
	$(INC)/sys/xtproto.h \
	$(INC)/sys/xt.h \
	$(FRC)
	$(CC) $(CFLAGS) -c xt.c

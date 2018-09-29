#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/lib.io.mk	10.7.1.3"
ROOT =
STRIP = strip
INC = $(ROOT)/usr/include

LIBNAME = ../lib.io
DASHO = -O
CFLAGS= $(DASHO) -I$(INC) -DINKERNEL $(MORECPP)
FRC =

FILES =\
	$(LIBNAME)(tty.o)\
	$(LIBNAME)(tt1.o)\
	$(LIBNAME)(clist.o)\
	$(LIBNAME)(partab.o)\
	$(LIBNAME)(stream.o)

all:	$(LIBNAME)

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)

.c.a:
	$(CC) -c $(CFLAGS) $<;\
	ar rv $@ $*.o
	-rm -f $*.o

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

FRC:
	-rm -f $(LIBNAME)
	ar qc $(LIBNAME)

#
# Header dependencies
#

$(LIBNAME)(clist.o): clist.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/var.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(partab.o): partab.c \
	$(FRC)

$(LIBNAME)(stream.o): stream.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/strstat.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/map.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(tt1.o): tt1.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(tty.o): tty.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/ttold.h \
	$(INC)/sys/file.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/inline.h \
	$(FRC)

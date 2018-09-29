#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/lib.io.mk	10.7.1.1"
ROOT =
STRIP = strip
INC = $(ROOT)/usr/include

LIBNAME = ../lib.io
DASHG =
DASHO = -O
PFLAGS= $(DASHG) -I$(INC)
CFLAGS= $(DASHO) $(PFLAGS) -DINKERNEL $(MORECPP)
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
	$(INC)/sys/conf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(tt1.o): tt1.c \
	$(INC)/sys/conf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/region.h \
	$(INC)/sys/inline.h \
	$(FRC)

$(LIBNAME)(tty.o): tty.c \
	$(INC)/sys/conf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/ttold.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/region.h \
	$(INC)/sys/inline.h \
	$(FRC)

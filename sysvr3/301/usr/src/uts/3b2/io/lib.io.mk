#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/lib.io.mk	10.7"
ROOT =
STRIP = strip
INCRT = $(ROOT)/usr/include

LIBNAME = ../lib.io
DASHG =
DASHO = -O
PFLAGS= $(DASHG) -I$(INCRT)
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
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(partab.o): partab.c \
	$(FRC)

$(LIBNAME)(stream.o): stream.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(tt1.o): tt1.c \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

$(LIBNAME)(tty.o): tty.c \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

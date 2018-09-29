#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/ports.mk	10.10.1.3"
ROOT =
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d
DASHO = -O
CFLAGS = $(DASHO) -I$(INC) -Uvax -Uu3b -Uu3b15 -Du3b2 -DINKERNEL -DDBO -Updp11
FRC =

DFILES =\
	ports.o


OPFILES = \
	lla_ppc|\
	ppc

install:	drivers

drivers:	all

all:	PORTS

.c.o:
	@-case $* in \
	$(OPFILES))\
		echo "\t$(CC) $(CFLAGS) -c $<";\
		$(CC) $(CFLAGS) -c $<;\
		exit $$?;\
		;;\
	*)\
		echo "\t$(CC) -c $(CFLAGS) $<";\
		$(CC) -c $(CFLAGS) $<;\
		;;\
	esac

clean:
	-rm -f ppc.o lla_ppc.o ports.o

clobber:	clean

PORTS:	$(DFILES) $(MASTERD)/ports
	$(MKBOOT) -m $(MASTERD) -d . ports.o;

ports.o:	lla_ppc.o ppc.o
	-$(LD) -r ppc.o lla_ppc.o -o ports.o

FRC:

ppc.o: ppc.c \
	$(INC)/sys/errno.h \
	$(INC)/sys/cio_defs.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/devcode.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/file.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/iu.h \
	$(INC)/sys/lla_ppc.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/pp_dep.h \
	$(INC)/sys/ppc.h \
	$(INC)/sys/ppc_lla.h \
	$(INC)/sys/pump.h \
	$(INC)/sys/queue.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/inline.h \
	$(FRC)

lla_ppc.o: lla_ppc.c \
	$(INC)/sys/cio_defs.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/lla_ppc.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/pp_dep.h \
	$(INC)/sys/ppc.h \
	$(INC)/sys/ppc_lla.h \
	$(INC)/sys/queue.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/firmware.h \
	$(FRC)

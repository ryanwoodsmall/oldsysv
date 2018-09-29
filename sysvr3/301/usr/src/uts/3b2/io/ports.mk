#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/ports.mk	10.9"
ROOT =
INCRT = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d
CFLAGS = -O -I$(INCRT) -Uvax -Uu3b -Uu3b5 -Du3b2 -DINKERNEL -DDBO -Updp11
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
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cio_defs.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/devcode.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/iu.h \
	$(INCRT)/sys/lla_ppc.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/pp_dep.h \
	$(INCRT)/sys/ppc.h \
	$(INCRT)/sys/ppc_lla.h \
	$(INCRT)/sys/pump.h \
	$(INCRT)/sys/queue.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

lla_ppc.o: lla_ppc.c \
	$(INCRT)/sys/cio_defs.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/lla_ppc.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/pp_dep.h \
	$(INCRT)/sys/ppc.h \
	$(INCRT)/sys/ppc_lla.h \
	$(INCRT)/sys/queue.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

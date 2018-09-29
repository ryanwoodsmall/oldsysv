#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/drivers.mk	10.11"
ROOT =
STRIP = strip
INCRT = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d

DASHG = 
DASHO = -O 
PFLAGS= $(DASHG) -I$(INCRT) -DINKERNEL $(MORECPP)
CFLAGS= $(DASHO) -Uvax -Updp11 -Uu3b -Uu3b5 -Du3b2 $(PFLAGS)
FRC =

DFILES =\
	iuart.o\
	idisk.o\
	hdelog.o\
	log.o \
	clone.o \
	sp.o \
	timod.o \
	tirdwr.o\
	mem.o\
	gentty.o\
	msg.o\
	prf.o\
	sem.o\
	shm.o\
	sxt.o\
	stubs.o\
	ipc.o \
	mau.o

drivers:	all

all:	IUART \
	IDISK \
	HDELOG \
	LOG  \
	CLONE  \
	SP  \
	TIMOD  \
	TIRDWR \
	MEM \
	GENTTY \
	MSG \
	PRF \
	SEM \
	SHM \
	SXT \
	STUBS \
	IPC \
	MAU

IUART:	iuart.o $(MASTERD)/iuart
	$(MKBOOT) -m $(MASTERD) -d . iuart.o;

iuart.o: iu.o $(FRC)
	-@if [ "$(VPATH)" ]; then cp iu.o $@; else ln iu.o $@; fi

HDELOG:	hdelog.o $(MASTERD)/hdelog
	$(MKBOOT) -m $(MASTERD) -d . hdelog.o; 

hdelog.o: hde.o $(FRC)
	-@if [ "$(VPATH)" ]; then cp hde.o $@; else ln hde.o $@; fi

IDISK:	idisk.o $(MASTERD)/idisk
	$(MKBOOT) -m $(MASTERD) -d . idisk.o; 

idisk.o: id.o if.o sddrv.o physdsk.o $(FRC)
	$(LD) -r $(LDFLAGS) -o idisk.o id.o if.o sddrv.o physdsk.o

clean:
	-rm -f $(DFILES)

clobber:	clean

FRC:

#
# Header dependencies
#

hde.o: hde.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/hdeioctl.h \
	$(INCRT)/sys/hdelog.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)

id.o: id.c \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/hdelog.h \
	$(INCRT)/sys/id.h \
	$(INCRT)/sys/if.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

if.o: if.c \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/if.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/iu.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/file.h \
	$(FRC)

physdsk.o: physdsk.c \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

IPC:	ipc.o $(MASTERD)/ipc
	$(MKBOOT) -m $(MASTERD) -d . ipc.o; 

ipc.o: ipc.c \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(FRC)
	$(CC) $(CFLAGS) -c ipc.c

iu.o: iu.c \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/iu.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
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

SP:	sp.o $(MASTERD)/sp
	$(MKBOOT) -m $(MASTERD) -d . sp.o;

sp.o: sp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sp.c

CLONE:	clone.o $(MASTERD)/clone
	$(MKBOOT) -m $(MASTERD) -d . clone.o;

clone.o: clone.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/inode.h \
	$(FRC)
	$(CC) $(CFLAGS) -c clone.c

LOG:	log.o $(MASTERD)/log
	$(MKBOOT) -m $(MASTERD) -d . log.o;

log.o: log.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/log.h \
	$(FRC)
	$(CC) $(CFLAGS) -c log.c

TIMOD:	timod.o $(MASTERD)/timod
	$(MKBOOT) -m $(MASTERD) -d . timod.o;

timod.o: timod.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(FRC)
	$(CC) $(CFLAGS) -c timod.c

TIRDWR:	tirdwr.o $(MASTERD)/tirdwr
	$(MKBOOT) -m $(MASTERD) -d . tirdwr.o;

tirdwr.o: tirdwr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(FRC)
	$(CC) $(CFLAGS) -c tirdwr.c

MEM:	mem.o $(MASTERD)/mem
	$(MKBOOT) -m $(MASTERD) -d . mem.o;

mem.o: mem.c \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/iobd.h \
	$(FRC)
	$(CC) $(CFLAGS) -c mem.c

MSG:	msg.o $(MASTERD)/msg
	$(MKBOOT) -m $(MASTERD) -d . msg.o;

msg.o: msg.c \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/msg.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/region.h \
	$(FRC)
	$(CC) $(CFLAGS) -c msg.c

PRF:	prf.o $(MASTERD)/prf
	$(MKBOOT) -m $(MASTERD) -d . prf.o;

prf.o: prf.c \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(FRC)
	$(CC) $(CFLAGS) -c prf.c

sddrv.o: sddrv.c \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(FRC)

SEM:	sem.o $(MASTERD)/sem
	$(MKBOOT) -m $(MASTERD) -d . sem.o;

sem.o: sem.c \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/sem.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/region.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sem.c

SHM:	shm.o $(MASTERD)/shm
	$(MKBOOT) -m $(MASTERD) -d . shm.o;

shm.o: shm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/shm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sbd.h \
	$(FRC)
	$(CC) $(CFLAGS) -c shm.c

STUBS:	stubs.o $(MASTERD)/stubs
	$(MKBOOT) -m $(MASTERD) -d . stubs.o;

stubs.o: stubs.c \
	$(FRC)
	$(CC) $(CFLAGS) -c stubs.c

SXT:	sxt.o $(MASTERD)/sxt
	$(MKBOOT) -m $(MASTERD) -d . sxt.o;

sxt.o: sxt.c \
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
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sxt.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/region.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sxt.c

GENTTY:	gentty.o $(MASTERD)/gentty
	$(MKBOOT) -m $(MASTERD) -d . gentty.o; 

gentty.o: gentty.c \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/region.h \
	$(FRC)
	$(CC) $(CFLAGS) -c gentty.c

MAU:	mau.o $(MASTERD)/mau
	$(MKBOOT) -m $(MASTERD) -d . mau.o;

mau.o:	mau.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/mau.h \
	$(FRC)
	$(CC) $(CFLAGS) -c mau.c

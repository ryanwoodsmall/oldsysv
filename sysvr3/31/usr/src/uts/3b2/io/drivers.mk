#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/drivers.mk	10.12.1.3"
ROOT =
STRIP = strip
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d

DASHG = 
DASHO = -O 
PFLAGS= $(DASHG) -I$(INC) -DINKERNEL $(MORECPP)
CFLAGS= $(DASHO) -Uvax -Updp11 -Uu3b -Uu3b15 -Du3b2 $(PFLAGS)
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
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/hdeioctl.h \
	$(INC)/sys/hdelog.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/open.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/region.h \
	$(INC)/sys/user.h \
	$(INC)/sys/vtoc.h \
	$(FRC)

id.o: id.c \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/dma.h \
	$(INC)/sys/elog.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/hdelog.h \
	$(INC)/sys/id.h \
	$(INC)/sys/if.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/open.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/vtoc.h \
	$(INC)/sys/inline.h \
	$(FRC)

if.o: if.c \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/dma.h \
	$(INC)/sys/elog.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/if.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/iu.h \
	$(INC)/sys/open.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/vtoc.h \
	$(INC)/sys/region.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/file.h \
	$(FRC)

physdsk.o: physdsk.c \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/elog.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/region.h \
	$(INC)/sys/inline.h \
	$(FRC)

IPC:	ipc.o $(MASTERD)/ipc
	$(MKBOOT) -m $(MASTERD) -d . ipc.o; 

ipc.o: ipc.c \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/ipc.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(FRC)
	$(CC) $(CFLAGS) -c ipc.c

iu.o: iu.c \
	$(INC)/sys/conf.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/dma.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/iu.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
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

SP:	sp.o $(MASTERD)/sp
	$(MKBOOT) -m $(MASTERD) -d . sp.o;

sp.o: sp.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/stream.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sp.c

CLONE:	clone.o $(MASTERD)/clone
	$(MKBOOT) -m $(MASTERD) -d . clone.o;

clone.o: clone.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/ioctl.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/inode.h \
	$(FRC)
	$(CC) $(CFLAGS) -c clone.c

LOG:	log.o $(MASTERD)/log
	$(MKBOOT) -m $(MASTERD) -d . log.o;

log.o: log.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/ioctl.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/strlog.h \
	$(INC)/sys/log.h \
	$(FRC)
	$(CC) $(CFLAGS) -c log.c

TIMOD:	timod.o $(MASTERD)/timod
	$(MKBOOT) -m $(MASTERD) -d . timod.o;

timod.o: timod.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/tihdr.h \
	$(INC)/sys/timod.h \
	$(INC)/sys/tiuser.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(FRC)
	$(CC) $(CFLAGS) -c timod.c

TIRDWR:	tirdwr.o $(MASTERD)/tirdwr
	$(MKBOOT) -m $(MASTERD) -d . tirdwr.o;

tirdwr.o: tirdwr.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/tihdr.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/errno.h \
	$(FRC)
	$(CC) $(CFLAGS) -c tirdwr.c

MEM:	mem.o $(MASTERD)/mem
	$(MKBOOT) -m $(MASTERD) -d . mem.o;

mem.o: mem.c \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/iobd.h \
	$(FRC)
	$(CC) $(CFLAGS) -c mem.c

MSG:	msg.o $(MASTERD)/msg
	$(MKBOOT) -m $(MASTERD) -d . msg.o;

msg.o: msg.c \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/ipc.h \
	$(INC)/sys/map.h \
	$(INC)/sys/msg.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/region.h \
	$(FRC)
	$(CC) $(CFLAGS) -c msg.c

PRF:	prf.o $(MASTERD)/prf
	$(MKBOOT) -m $(MASTERD) -d . prf.o;

prf.o: prf.c \
	$(INC)/sys/buf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(FRC)
	$(CC) $(CFLAGS) -c prf.c

sddrv.o: sddrv.c \
	$(INC)/sys/buf.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	$(FRC)

SEM:	sem.o $(MASTERD)/sem
	$(MKBOOT) -m $(MASTERD) -d . sem.o;

sem.o: sem.c \
	$(INC)/sys/buf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/ipc.h \
	$(INC)/sys/map.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/sem.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/region.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sem.c

SHM:	shm.o $(MASTERD)/shm
	$(MKBOOT) -m $(MASTERD) -d . shm.o;

shm.o: shm.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/ipc.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/shm.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/tuneable.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/sbd.h \
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
	$(INC)/sys/signal.h \
	$(INC)/sys/sxt.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/ttold.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/region.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sxt.c

GENTTY:	gentty.o $(MASTERD)/gentty
	$(MKBOOT) -m $(MASTERD) -d . gentty.o; 

gentty.o: gentty.c \
	$(INC)/sys/conf.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	$(INC)/sys/region.h \
	$(FRC)
	$(CC) $(CFLAGS) -c gentty.c

MAU:	mau.o $(MASTERD)/mau
	$(MKBOOT) -m $(MASTERD) -d . mau.o;

mau.o:	mau.c \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/user.h \
	$(INC)/sys/mau.h \
	$(FRC)
	$(CC) $(CFLAGS) -c mau.c

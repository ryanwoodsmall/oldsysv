#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/drivers.mk	10.12.1.5"
ROOT =
STRIP = strip
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d

DASHO = -O 
PFLAGS= -I$(INC) -DINKERNEL $(MORECPP)
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

IDISK:	idisk.o $(MASTERD)/idisk
	$(MKBOOT) -m $(MASTERD) -d . idisk.o; 

HDELOG:	hdelog.o $(MASTERD)/hdelog
	$(MKBOOT) -m $(MASTERD) -d . hdelog.o; 

LOG:	log.o $(MASTERD)/log
	$(MKBOOT) -m $(MASTERD) -d . log.o;

CLONE:	clone.o $(MASTERD)/clone
	$(MKBOOT) -m $(MASTERD) -d . clone.o;

SP:	sp.o $(MASTERD)/sp
	$(MKBOOT) -m $(MASTERD) -d . sp.o;

TIMOD:	timod.o $(MASTERD)/timod
	$(MKBOOT) -m $(MASTERD) -d . timod.o;

TIRDWR:	tirdwr.o $(MASTERD)/tirdwr
	$(MKBOOT) -m $(MASTERD) -d . tirdwr.o;

MEM:	mem.o $(MASTERD)/mem
	$(MKBOOT) -m $(MASTERD) -d . mem.o;

GENTTY:	gentty.o $(MASTERD)/gentty
	$(MKBOOT) -m $(MASTERD) -d . gentty.o; 

MSG:	msg.o $(MASTERD)/msg
	$(MKBOOT) -m $(MASTERD) -d . msg.o;

PRF:	prf.o $(MASTERD)/prf
	$(MKBOOT) -m $(MASTERD) -d . prf.o;

SEM:	sem.o $(MASTERD)/sem
	$(MKBOOT) -m $(MASTERD) -d . sem.o;

SHM:	shm.o $(MASTERD)/shm
	$(MKBOOT) -m $(MASTERD) -d . shm.o;

SXT:	sxt.o $(MASTERD)/sxt
	$(MKBOOT) -m $(MASTERD) -d . sxt.o;

STUBS:	stubs.o $(MASTERD)/stubs
	$(MKBOOT) -m $(MASTERD) -d . stubs.o;

IPC:	ipc.o $(MASTERD)/ipc
	$(MKBOOT) -m $(MASTERD) -d . ipc.o; 

MAU:	mau.o $(MASTERD)/mau
	$(MKBOOT) -m $(MASTERD) -d . mau.o;

iuart.o: iu.o $(FRC)
	-@if [ "$(VPATH)" ]; then cp iu.o $@; else ln iu.o $@; fi

hdelog.o: hde.o $(FRC)
	-@if [ "$(VPATH)" ]; then cp hde.o $@; else ln hde.o $@; fi

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
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/file.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/vtoc.h \
	$(INC)/sys/hdelog.h \
	$(INC)/sys/hdeioctl.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/open.h \
	$(FRC)

id.o: id.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/id.h \
	$(INC)/sys/if.h \
	$(INC)/sys/dma.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/elog.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/vtoc.h \
	$(INC)/sys/hdelog.h \
	$(INC)/sys/open.h \
	$(INC)/sys/inline.h \
	$(FRC)

if.o: if.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/dma.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/iu.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/region.h \
	$(INC)/sys/user.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/elog.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/if.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/vtoc.h \
	$(INC)/sys/open.h \
	$(INC)/sys/file.h \
	$(INC)/sys/tuneable.h \
	$(FRC)

physdsk.o: physdsk.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/region.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/elog.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/inline.h \
	$(FRC)

ipc.o: ipc.c \
	$(INC)/sys/errno.h \
	$(INC)/sys/types.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/ipc.h \
	$(FRC)
	$(CC) $(CFLAGS) -c ipc.c

iu.o: iu.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/iu.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/dma.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

sp.o: sp.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/stream.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sp.c

clone.o: clone.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/file.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(FRC)
	$(CC) $(CFLAGS) -c clone.c

log.o: log.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/strstat.h \
	$(INC)/sys/log.h \
	$(INC)/sys/strlog.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/inline.h \
	$(FRC)
	$(CC) $(CFLAGS) -c log.c

timod.o: timod.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
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

mem.o: mem.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/iobd.h \
	$(FRC)
	$(CC) $(CFLAGS) -c mem.c

msg.o: msg.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/map.h \
	$(INC)/sys/ipc.h \
	$(INC)/sys/msg.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/sysinfo.h \
	$(FRC)
	$(CC) $(CFLAGS) -c msg.c

prf.o: prf.c \
	$(INC)/sys/signal.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(FRC)
	$(CC) $(CFLAGS) -c prf.c

sddrv.o: sddrv.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/csr.h \
	$(FRC)

sem.o: sem.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/map.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/ipc.h \
	$(INC)/sys/sem.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/sysinfo.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sem.c

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

stubs.o: stubs.c \
	$(FRC)
	$(CC) $(CFLAGS) -c stubs.c

sxt.o: sxt.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/page.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/macro.h \
	$(INC)/sys/istk.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/file.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/ttold.h \
	$(INC)/sys/sxt.h \
	$(INC)/sys/inline.h \
	$(FRC)
	$(CC) $(CFLAGS) -c sxt.c

gentty.o: gentty.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/tty.h \
	$(INC)/sys/stream.h \
	$(FRC)
	$(CC) $(CFLAGS) -c gentty.c

mau.o: mau.c \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/reg.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/user.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/mau.h \
	$(FRC)
	$(CC) $(CFLAGS) -c mau.c

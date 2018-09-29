#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:nudnix/nudnix.mk	10.15"
#
#		Copyright 1984 AT&T
#

ROOT = 
STRIP = strip
INCRT = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d
DASHG = 
VER = mip
DASHO = -O
DUDEBUG = YES
CFLAGS= -I$(INCRT) -DINKERNEL $(MORECPP) $(DASHG) $(DASHO) -Du3b2 -DDUDEBUG=$(DUDEBUG)
PFLAGS= -I$(INCRT) -DINKERNEL $(MORECPP) $(DASHG)
FRC =

DFILES =\
	du.o

FILES =\
	adv.o\
	auth.o\
	canon.o\
	cirmgr.o\
	comm.o\
	fileop.o\
	fumount.o\
	netboot.o\
	que.o\
	queue.o\
	rdebug.o\
	recover.o\
	remcall.o\
	rfadmin.o\
	rfcanon.o\
	rfsys.o\
	rmount.o\
	rmove.o\
	rnami.o\
	rsc.o\
	serve.o\
	string.o

#
# Files for avoiding optimization
#

NOPFILES = \
	streamio | \
	prf

all:	DU

DU:	$(DFILES) $(MASTERD)/du
	$(MKBOOT) -m $(MASTERD) -d . du.o

du.o:	$(FILES)
	$(LD) -r -o du.o $(FILES)

clean:
	-rm -f *.o

clobber:	clean

FRC:

#
# Header dependencies
#

adv.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/adv.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/inline.h\
	$(FRC)

auth.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/stat.h\
	$(INCRT)/sys/idtab.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/inline.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

canon.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(FRC)

cirmgr.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/stropts.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/recover.h\
	$(INCRT)/sys/inline.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/tihdr.h\
	$(FRC)

comm.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

fileop.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/sbd.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/stat.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

fumount.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/adv.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/recover.h\
	$(FRC)

netboot.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/adv.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/recover.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

que.o:\
	$(INCRT)/sys/que.h\
	$(FRC)

queue.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/recover.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

rdebug.o:\
	$(INCRT)/sys/inline.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

recover.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/fstyp.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/recover.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rfsys.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/sys/fcntl.h\
	$(INCRT)/sys/flock.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/proc.h\
	$(FRC)

remcall.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/stat.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/idtab.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/inline.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/nami.h\
	$(INCRT)/sys/fstyp.h\
	$(FRC)

rfadmin.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/recover.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/adv.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/rfsys.h\
	$(INCRT)/sys/tihdr.h\
	$(FRC)

rfcanon.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/dirent.h\
	$(INCRT)/sys/hetero.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

rfsys.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/sbd.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/recover.h\
	$(INCRT)/sys/rfsys.h\
	$(FRC)

rmount.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/adv.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/inline.h\
	$(INCRT)/sys/sysinfo.h\
	$(FRC)

rmove.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

rnami.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/inline.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

rsc.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/rdebug.h\
	$(FRC)

serve.o:\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sema.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/sys/fs/s5inode.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/comm.h\
	$(INCRT)/sys/psw.h\
	$(INCRT)/sys/pcb.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/nserve.h\
	$(INCRT)/sys/cirmgr.h\
	$(INCRT)/sys/message.h\
	$(INCRT)/sys/mount.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/fs/s5filsys.h\
	$(INCRT)/sys/fstyp.h\
	$(INCRT)/sys/fcntl.h\
	$(INCRT)/sys/nami.h\
	$(INCRT)/sys/region.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/stat.h\
	$(INCRT)/sys/sysinfo.h\
	$(INCRT)/sys/idtab.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/inline.h\
	$(INCRT)/sys/rdebug.h\
	$(INCRT)/sys/cmn_err.h\
	$(INCRT)/sys/conf.h\
	$(FRC)

string.o:\
	$(FRC)


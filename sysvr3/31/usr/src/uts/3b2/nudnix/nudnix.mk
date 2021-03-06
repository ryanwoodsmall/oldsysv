#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:nudnix/nudnix.mk	10.15.6.3"
#
#		Copyright 1984 AT&T
#

ROOT = 
STRIP = strip
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d
DASHG = 
VER = mip
DASHO = -O
DUDEBUG = YES
CFLAGS= -I$(INC) -DINKERNEL $(MORECPP) $(DASHG) $(DASHO) -Du3b2 -DDUDEBUG=$(DUDEBUG)
PFLAGS= -I$(INC) -DINKERNEL $(MORECPP) $(DASHG)
FRC =

DFILES =\
	du.o

FILES =\
	adv.o\
	auth.o\
	cache.o\
	canon.o\
	cirmgr.o\
	comm.o\
	fileop.o\
	fumount.o\
	netboot.o\
	que.o\
	queue.o\
	rbio.o\
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
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/adv.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/inline.h\
	$(FRC)

auth.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/file.h\
	$(INC)/sys/stat.h\
	$(INC)/sys/idtab.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/inline.h\
	$(INC)/sys/rdebug.h\
	$(FRC)

cache.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/message.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/buf.h\
	$(INC)/sys/rbuf.h\
	$(INC)/sys/fcntl.h\
	$(INC)/sys/flock.h\
	$(FRC)

canon.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/param.h\
	$(FRC)

cirmgr.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/file.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/stropts.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/var.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/recover.h\
	$(INC)/sys/inline.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/tihdr.h\
	$(FRC)

comm.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/region.h\
	$(INC)/sys/user.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/file.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/message.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/recover.h\
	$(FRC)

fileop.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/param.h\
	$(INC)/sys/systm.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/file.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/sbd.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/var.h\
	$(INC)/sys/stat.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/buf.h\
	$(INC)/sys/rbuf.h\
	$(FRC)

fumount.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/var.h\
	$(INC)/sys/adv.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/recover.h\
	$(FRC)

netboot.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/var.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/adv.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/recover.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(FRC)

que.o:\
	$(INC)/sys/que.h\
	$(FRC)

queue.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/param.h\
	$(INC)/sys/systm.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/region.h\
	$(INC)/sys/recover.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(FRC)

rbio.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/sbd.h\
	$(INC)/sys/param.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/systm.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/user.h\
	$(INC)/sys/rbuf.h\
	$(INC)/sys/buf.h\
	$(INC)/sys/iobuf.h\
	$(INC)/sys/conf.h\
	$(INC)/sys/var.h\
	$(INC)/sys/cmn_err.h\
	$(INC)/sys/inline.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/message.h\
	$(FRC)

rdebug.o:\
	$(INC)/sys/inline.h\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/rdebug.h\
	$(FRC)

recover.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/file.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/param.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/fstyp.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/var.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/recover.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rfsys.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/conf.h\
	$(INC)/sys/fcntl.h\
	$(INC)/sys/flock.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(FRC)

remcall.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/param.h\
	$(INC)/sys/systm.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/file.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/var.h\
	$(INC)/sys/stat.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/idtab.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/inline.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/nami.h\
	$(INC)/sys/fstyp.h\
	$(INC)/sys/rbuf.h\
	$(INC)/sys/buf.h\
	$(FRC)

rfadmin.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/param.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/region.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/file.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/var.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/recover.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/adv.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/rfsys.h\
	$(INC)/sys/tihdr.h\
	$(FRC)

rfcanon.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/message.h\
	$(INC)/sys/dirent.h\
	$(INC)/sys/hetero.h\
	$(INC)/sys/fcntl.h\
	$(INC)/sys/file.h\
	$(INC)/sys/rdebug.h\
	$(FRC)

rfsys.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/sbd.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/recover.h\
	$(INC)/sys/rfsys.h\
	$(FRC)

rmount.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/var.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/adv.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/inline.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/buf.h\
	$(FRC)

rmove.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/rdebug.h\
	$(FRC)

rnami.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/param.h\
	$(INC)/sys/systm.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/file.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/inline.h\
	$(INC)/sys/rdebug.h\
	$(FRC)

rsc.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/message.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/rbuf.h\
	$(INC)/sys/buf.h\
	$(FRC)

serve.o:\
	$(INC)/sys/types.h\
	$(INC)/sys/sema.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/param.h\
	$(INC)/sys/systm.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/comm.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/user.h\
	$(INC)/sys/nserve.h\
	$(INC)/sys/cirmgr.h\
	$(INC)/sys/message.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/var.h\
	$(INC)/sys/file.h\
	$(INC)/sys/fstyp.h\
	$(INC)/sys/fcntl.h\
	$(INC)/sys/nami.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/stat.h\
	$(INC)/sys/sysinfo.h\
	$(INC)/sys/idtab.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/inline.h\
	$(INC)/sys/rdebug.h\
	$(INC)/sys/cmn_err.h\
	$(INC)/sys/conf.h\
	$(INC)/sys/buf.h\
	$(INC)/sys/rbuf.h\
	$(FRC)

string.o:\
	$(FRC)


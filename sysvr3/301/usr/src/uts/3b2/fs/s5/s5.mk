#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:fs/s5/s5.mk	10.10"
ROOT = 
STRIP = strip
INCRT = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../../master.d
CC=cc

DASHG = 
DASHO = -O
PFLAGS = $(DASHG) -DINKERNEL $(MORECPP)
#CFLAGS= $(DASHO) -Uvax -Uu3b -Uu3b5 -Du3b2 $(PFLAGS)
CFLAGS= $(DASHO) $(PFLAGS)
DEFLIST=
FRC =

FILES =\
	s5alloc.o \
	s5blklist.o \
	s5iget.o \
	s5nami.o \
	s5pipe.o \
	s5rdwri.o \
	s5subr.o \
	s5sys2.o \
	s5sys3.o 
all:	S5

S5:	s5.o $(MASTERD)/s5
	$(MKBOOT) -m $(MASTERD) -d . s5.o

s5.o: $(FILES)
	$(LD) -r -o s5.o $(FILES)

.c.o:
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.c



clean:
	-rm -f *.o

clobber:	clean
	-rm -f s5 S5


#
# Header dependencies
#
s5alloc.o: s5alloc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5fblk.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

s5blklist.o: s5blklist.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

s5iget.o: s5iget.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

s5nami.o: s5nami.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

s5pipe.o: s5pipe.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

s5rdwri.o: s5rdwri.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

s5subr.o: s5subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

s5sys2.o: s5sys2.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/conf.h \
	$(FRC)
s5sys3.o: s5sys3.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

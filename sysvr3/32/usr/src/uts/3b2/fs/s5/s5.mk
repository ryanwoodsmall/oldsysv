#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:fs/s5/s5.mk	10.11.1.4"
ROOT = 
STRIP = strip
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../../master.d
CC=cc

DASHO = -O
PFLAGS = -DINKERNEL $(MORECPP)
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
	$(CC) $(DEFLIST) -I$(INC) $(CFLAGS) -c $*.c



clean:
	-rm -f *.o

clobber:	clean
	-rm -f s5 S5

FRC:

#
# Header dependencies
#

s5alloc.o: s5alloc.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/fs/s5fblk.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/ino.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/user.h \
	$(INC)/sys/var.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(FRC)

s5blklist.o: s5blklist.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/pfdat.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(FRC)

s5iget.o: s5iget.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/ino.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

s5nami.o: s5nami.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/file.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(FRC)

s5pipe.o: s5pipe.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/file.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/cmn_err.h \
	$(FRC)

s5rdwri.o: s5rdwri.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/file.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/flock.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/debug.h \
	$(FRC)

s5subr.o: s5subr.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/ustat.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/file.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/statfs.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/open.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/flock.h \
	$(FRC)

s5sys2.o: s5sys2.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/dirent.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/nami.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(FRC)

s5sys3.o: s5sys3.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/ino.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/file.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/flock.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/statfs.h \
	$(INC)/sys/ioctl.h \
	$(INC)/sys/var.h \
	$(INC)/sys/ttold.h \
	$(INC)/sys/open.h \
	$(INC)/sys/debug.h \
	$(FRC)

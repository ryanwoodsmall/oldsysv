#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)3b2s52k:fs/s52k/s52k.mk	10.3"
ROOT = 
STRIP = strip
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../../master.d
CC=cc
DASHO = -O
PFLAGS = -DINKERNEL $(MORECPP)
CFLAGS= $(DASHO) -Uvax -Uu3b -Uu3b5 -Du3b2 -DFsTYPE=4 $(PFLAGS)
DEFLIST=
FRC =

FILES =\
	s52kalloc.o \
	s52kblklist.o \
	s52kiget.o \
	s52knami.o \
	s52kpipe.o \
	s52krdwri.o \
	s52ksubr.o \
	s52ksys2.o \
	s52ksys3.o \
	s52kbio.o

all:	S52K

S52K:	s52k.o $(MASTERD)/s52k
	$(MKBOOT) -m $(MASTERD) -d . s52k.o

s52k.o: $(FILES)
	$(LD) -r -o s52k.o $(FILES)

.c.o:
	$(CC) $(DEFLIST) -I$(INC) $(CFLAGS) -c $*.c



clean:
	-rm -f *.o

clobber:	clean
	-rm -f s52k.o S52K


#
# Header dependencies
#

s52kalloc.o: s52kalloc.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/fs/s5fblk.h \
	$(INC)/sys/fs/s5macros.h \
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

s52kblklist.o: s52kblklist.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/param.h \
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

s52kiget.o: s52kiget.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/fstyp.h \
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

s52knami.o: s52knami.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
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

s52kpipe.o: s52kpipe.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
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

s52krdwri.o: s52krdwri.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fstyp.h \
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
	$(INC)/sys/fs/s5macros.h \
	$(FRC)

s52ksubr.o: s52ksubr.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fstyp.h \
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

s52ksys2.o: s52ksys2.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/dirent.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/cmn_err.h \
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

s52ksys3.o: s52ksys3.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
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
s52kbio.o: s52kbio.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/iobuf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/inline.h \
	$(FRC)

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:fs/du/du.mk	10.5.4.5"
ROOT = 
STRIP = strip
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../../master.d
CC=cc

DASHO = -O
DUDEBUG = YES
CFLAGS= $(DASHO) -DDUDEBUG=$(DUDEBUG) -DINKERNEL $(MORECPP)
DEFLIST=
FRC =

FILES =\
	duiget.o \
	dusubr.o \
	dusys3.o \
	durdwri.o 

all:	DUFST

DUFST:	dufst.o $(MASTERD)/dufst
	$(MKBOOT) -m $(MASTERD) -d . dufst.o

dufst.o: $(FILES)
	$(LD) -r -o dufst.o $(FILES)

.c.o:
	$(CC) $(DEFLIST) -I$(INC) $(CFLAGS) -c $*.c



clean:
	-rm -f *.o

clobber:	clean
	-rm -f dufst DUFST

FRC:

#
# Header dependencies
#

duiget.o: duiget.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
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
	$(INC)/sys/file.h \
	$(INC)/sys/ino.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/var.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/region.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/rdebug.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/message.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/comm.h \
	$(FRC)

durdwri.o: durdwri.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
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
	$(INC)/sys/systm.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/rdebug.h \
	$(INC)/sys/message.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/comm.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/rbuf.h \
	$(FRC)

dusubr.o: dusubr.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/comm.h \
	$(INC)/sys/nserve.h \
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
	$(INC)/sys/debug.h \
	$(INC)/sys/rdebug.h \
	$(INC)/sys/message.h \
	$(FRC)

dusys3.o: dusys3.c \
	$(INC)/sys/types.h \
	$(INC)/sys/sema.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fstyp.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/comm.h \
	$(INC)/sys/nserve.h \
	$(INC)/sys/cirmgr.h \
	$(INC)/sys/ustat.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/file.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/inode.h \
	$(INC)/sys/nami.h \
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
	$(INC)/sys/debug.h \
	$(INC)/sys/rdebug.h \
	$(INC)/sys/message.h \
	$(INC)/sys/fcntl.h \
	$(FRC)

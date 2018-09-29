#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:fs/du/du.mk	10.4"
ROOT = 
STRIP = strip
INCRT = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../../master.d
CC=cc

DASHG = 
DASHO = -O
DUDEBUG = YES
PFLAGS = $(DASHG) -DINKERNEL $(MORECPP)
#CFLAGS= $(DASHO) -Uvax -Uu3b -Uu3b5 -Du3b2 $(PFLAGS)
CFLAGS= $(DASHO) $(PFLAGS) -DDUDEBUG=$(DUDEBUG)
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
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.c



clean:
	-rm -f *.o

clobber:	clean
	-rm -f dufst DUFST


#
# Header dependencies
#

duiget.o: duiget.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5param.h \
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
	$(INCRT)/sys/file.h \
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

durdwri.o: durdwri.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5param.h \
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
	$(INCRT)/sys/file.h \
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
dusubr.o: dusubr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5param.h \
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
	$(INCRT)/sys/file.h \
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

dusys3.o: dusys3.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5param.h \
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
	$(INCRT)/sys/file.h \
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

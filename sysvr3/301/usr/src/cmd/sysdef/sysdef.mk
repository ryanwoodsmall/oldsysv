#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sysdef-3b2:sysdef.mk	1.4"

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT)  -s
INS = install
FRC =

all: sysdef

install: sysdef
	$(INS) -n $(ROOT)/etc sysdef

sysdef: sysdef.c
	$(CC) $(CFLAGS) sysdef.c -lld -o sysdef $(LDLIBS)

clobber: clean

clean:
	rm -f sysdef

FRC:

#
# Header dependencies
#

sysdef: sysdef.c \
	$(INCRT)/a.out.h \
	$(INCRT)/aouthdr.h \
	$(INCRT)/ctype.h \
	$(INCRT)/filehdr.h \
	$(INCRT)/ldfcn.h \
	$(INCRT)/linenum.h \
	$(INCRT)/nlist.h \
	$(INCRT)/reloc.h \
	$(INCRT)/scnhdr.h \
	$(INCRT)/stdio.h \
	$(INCRT)/storclass.h \
	$(INCRT)/syms.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/msg.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sem.h \
	$(INCRT)/sys/shm.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/var.h \
	$(FRC)

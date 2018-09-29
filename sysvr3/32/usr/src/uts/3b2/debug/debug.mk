#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:debug/debug.mk	10.11.1.4"
#
# makefile for libc/m32
#


M4=m4 m4.defs
CC=cc
DASHO = -O
CFLAGS= $(DASHO) $(MORECPP)
PROF=
DEFLIST=
INC=$(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d
FRC=

OBJECTS=\
	arg.o \
	ctype.o \
	data.o \
	disasm.o \
	doprnt.o \
	prtabs.o \
	sprintf.o \
	tables.o \
	trace.o \
	utils.o

all:	DEBUG

DEBUG: debug.o $(MASTERD)/debug
	$(MKBOOT) -m $(MASTERD) -d . debug.o;

debug.o: $(OBJECTS)
	$(LD) -r -o debug.o $(OBJECTS)


.c.o:
	$(CC) $(DEFLIST) -I$(INC) $(CFLAGS) -c $*.c 

.s.o:
	$(M4) -DMCOUNT=#  $*.s   > $*.m.s
	$(CC) $(DEFLIST) -I$(INC) $(CFLAGS) -c $*.m.s
	mv $*.m.o $*.o
	rm $*.m.s
clean:
	-rm -f *.o

clobber: clean
	-rm -f debug DEBUG

FRC:

#
# Header dependencies
#

ctype.o: ctype.c \
	$(INC)/sys/ctype.h \
	$(FRC)

data.o: data.c \
	$(INC)/sys/stdio.h \
	$(FRC)

disasm.o: disasm.c \
	$(INC)/sys/types.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/disasm.h \
	$(INC)/sys/sgs.h \
	$(FRC)

doprnt.o: doprnt.s \
	$(FRC)

prtabs.o: prtabs.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/var.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/inline.h \
	$(FRC)

sprintf.o: sprintf.s \
	$(FRC)

tables.o: tables.c \
	$(INC)/sys/types.h \
	$(INC)/sys/disasm.h \
	$(FRC)

trace.o: trace.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/var.h \
	$(FRC)

utils.o: utils.c \
	$(INC)/sys/types.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/disasm.h \
	$(INC)/sys/sgs.h \
	$(FRC)

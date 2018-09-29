#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:debug/debug.mk	10.11"
#
# makefile for libc/m32
#


M4=m4 m4.defs
CC=cc
CFLAGS=-O $(MORECPP)
PROF=
DEFLIST=
INCRT=..
MKBOOT = mkboot
MASTERD = ../master.d

OBJECTS=\
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
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.c 

.s.o:
	$(M4) -DMCOUNT=#  $*.s   > $*.m.s
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.m.s
	mv $*.m.o $*.o
	rm $*.m.s
clean:
	-rm -f *.o

clobber: clean
	-rm -f debug DEBUG

ctype.o: ctype.c \
	$(INCRT)/sys/ctype.h \
	$(FRC)

data.o: data.c \
	$(INCRT)/sys/stdio.h \
	$(FRC)

disasm.o: disasm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/disasm.h \
	$(INCRT)/sys/sgs.h \
	$(FRC)

doprnt.o: doprnt.s \
	$(FRC)

prtabs.o: prtabs.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

trace.o: trace.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

sprintf.o: sprintf.s \
	$(FRC)

tables.o: tables.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/disasm.h \
	$(FRC)

utils.o: utils.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/disasm.h \
	$(INCRT)/sys/sgs.h \
	$(FRC)

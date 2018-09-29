#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:sys/makefile	1.5.1.7"
#
# makefile for libc/port/sys
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O
PROF=
NONPROF=
ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include/sys
INCLIBC=../../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)

OBJECTS=\
execl.o       execle.o      execv.o	lockf.o    \
msgsys.o      semsys.o      shmsys.o  

POBJECTS=\
execl.p       execle.p      execv.p	lockf.p    \
msgsys.p      semsys.p      shmsys.p  

nonprof: $(OBJECTS)

prof: $(POBJECTS)

msgsys.o msgsys.p semsys.o semsys.p \
	shmsys.o shmsys.p: $(INCSYS)/ipc.h
msgsys.o msgsys.p: $(INCSYS)/msg.h
semsys.o semsys.p: $(INCSYS)/sem.h
shmsys.o shmsys.p: $(INCSYS)/shm.h
msgsys.o msgsys.p semsys.o semsys.p \
	shmsys.o shmsys.p: $(INCSYS)/types.h
lockf.o lockf.p: $(INC)/errno.h 
lockf.o lockf.p: $(INC)/fcntl.h 
lockf.o lockf.p: $(INC)/unistd.h

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c  && mv $(*F).o $*.p

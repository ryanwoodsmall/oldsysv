#	@(#)makefile	1.2
#
# makefile for libc/port/sys
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O
ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include/sys

OBJECTS=\
execl.o       execle.o      execv.o       msgsys.o	\
semsys.o      shmsys.o      lockf.o

POBJECTS=\
execl.p       execle.p      execv.p       msgsys.p	\
semsys.p      shmsys.p      lockf.p

nonprof: $(OBJECTS)

prof: $(POBJECTS)

msgsys.o msgsys.p semsys.o semsys.p \
	shmsys.o shmsys.p: $(INCSYS)/ipc.h
msgsys.o msgsys.p: $(INCSYS)/msg.h
semsys.o semsys.p: $(INCSYS)/sem.h
shmsys.o shmsys.p: $(INCSYS)/shm.h
msgsys.o msgsys.p semsys.o semsys.p \
	shmsys.o shmsys.p: $(INCSYS)/types.h
lockf.o lockf.p: $(INC)/errno.h $(INC)/fcntl.h $(INC)/unistd.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC)  -c -p $(CFLAGS) $*.c    && mv $(*F).o $*.p
	$(CC) $(CFLAGS) -c $*.c

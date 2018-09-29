#	@(#)makefile	1.1
#
# makefile for libc/port/sys
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O

OBJECTS=\
execl.o       execle.o      execv.o       msgsys.o	\
semsys.o      shmsys.o  

POBJECTS=\
execl.p       execle.p      execv.p       msgsys.p	\
semsys.p      shmsys.p  

nonprof: $(OBJECTS)

prof: $(POBJECTS)

msgsys.o msgsys.p semsys.o semsys.p \
	shmsys.o shmsys.p: /usr/include/sys/ipc.h
msgsys.o msgsys.p: /usr/include/sys/msg.h
semsys.o semsys.p: /usr/include/sys/sem.h
shmsys.o shmsys.p: /usr/include/sys/shm.h
msgsys.o msgsys.p semsys.o semsys.p \
	shmsys.o shmsys.p: /usr/include/sys/types.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c    && mv $(*F).o $*.p
	$(CC) $(CFLAGS) -c $*.c

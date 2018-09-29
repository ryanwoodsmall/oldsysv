#	@(#)makefile	1.1
#
# makefile for libc/port/print
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O

OBJECTS=\
doprnt.o    fprintf.o   printf.o    sprintf.o

POBJECTS=\
doprnt.o    fprintf.o   printf.o    sprintf.o

nonprof: $(OBJECTS)

prof: $(POBJECTS)

doprnt.o doprnt.p: print.h
doprnt.o doprnt.p: /usr/include/ctype.h
doprnt.o doprnt.p fprintf.o fprintf.p \
	printf.o printf.p \
	sprintf.o sprintf.p: /usr/include/stdio.h
doprnt.o doprnt.p \
	sprintf.o sprintf.p: /usr/include/values.h
doprnt.o doprnt.p fprintf.o fprintf.p \
	printf.o printf.p \
	sprintf.o sprintf.p: /usr/include/varargs.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c    && mv $(*F).o $*.p
	$(CC) $(CFLAGS) -c $*.c

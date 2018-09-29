#	@(#)makefile	1.4
#
# makefile for libc/port/print
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O
ROOT=
INC=$(ROOT)/usr/include

OBJECTS=\
doprnt.o    fprintf.o   printf.o    sprintf.o  vfprintf.o  vprintf.o  vsprintf.o

POBJECTS=\
doprnt.o    fprintf.o   printf.o    sprintf.o  vfprintf.o  vprintf.o  vsprintf.o

nonprof: $(OBJECTS)

prof: $(POBJECTS)

doprnt.o doprnt.p: print.h
doprnt.o doprnt.p: $(INC)/ctype.h
doprnt.o doprnt.p fprintf.o fprintf.p \
	printf.o printf.p sprintf.o sprintf.p \
	vfprintf.o vfprintf.p vprintf.o vprintf.p \
	vsprintf.o vsprintf.p: $(INC)/stdio.h
doprnt.o doprnt.p \
	sprintf.o sprintf.p vsprintf.o vsprintf.p: $(INC)/values.h
doprnt.o doprnt.p fprintf.o fprintf.p \
	printf.o printf.p sprintf.o sprintf.p \
	vfprintf.o vfprintf.p vprintf.o vprintf.p \
	vsprintf.o vsprintf.p: $(INC)/varargs.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC) -c -p $(CFLAGS) $*.c    && mv $(*F).o $*.p
	$(CC) $(CFLAGS) -c $*.c

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:print/makefile	1.12"
#
# makefile for libc/port/print
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O
PROF=
NONPROF=
ROOT=
INC=$(ROOT)/usr/include
INCLIBC=../../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)

OBJECTS=\
doprnt.o    fprintf.o   printf.o    sprintf.o  vfprintf.o  vprintf.o  vsprintf.o

POBJECTS=\
doprnt.p    fprintf.p   printf.p    sprintf.p  vfprintf.p  vprintf.p  vsprintf.p

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
doprnt.o fprintf.o printf.o sprintf.o : $(INCLIBC)/shlib.h
.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c && mv $(*F).o $*.p

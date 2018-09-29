#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:stdio/makefile	1.14"
#
# makefile for libc/port/stdio
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
clrerr.o    ctermid.o	\
cuserid.o   data.o	doscan.o    fdopen.o	\
fgetc.o     fgets.o     filbuf.o    findiop.o	\
flsbuf.o    fopen.o     fputc.o     fputs.o	\
fread.o     fseek.o     ftell.o     fwrite.o	\
getchar.o   getpass.o   gets.o      getw.o	\
popen.o     putchar.o   puts.o      putw.o	\
rew.o       scanf.o     setbuf.o    setvbuf.o	\
stdio_def.o system.o	\
tempnam.o   tmpfile.o   tmpnam.o    ungetc.o

POBJECTS=\
clrerr.p    ctermid.p	\
cuserid.p   data.p      doscan.p    fdopen.p	\
fgetc.p     fgets.p     filbuf.p    findiop.p	\
flsbuf.p    fopen.p     fputc.p     fputs.p	\
fread.p     fseek.p     ftell.p     fwrite.p	\
getchar.p   getpass.p   gets.p      getw.p	\
popen.p     putchar.p   puts.p      putw.p	\
rew.p       scanf.p     setbuf.p    setvbuf.p	\
system.p    \
tempnam.p   tmpfile.p   tmpnam.p    ungetc.p

nonprof: $(OBJECTS)

prof: $(POBJECTS)

fgets.o fgets.p flsbuf.o flsbuf.p \
	fputs.o fputs.p fread.o fread.p \
	fwrite.o fwrite.p gets.o gets.p \
	puts.o puts.p: stdiom.h
doscan.o doscan.p: $(INC)/ctype.h
fopen.o fopen.p: $(INC)/fcntl.h
cuserid.o cuserid.p: $(INC)/pwd.h
getpass.o getpass.p \
	popen.o popen.p \
	system.o system.p: $(INC)/signal.h \
		$(INCSYS)/signal.h
clrerr.o clrerr.p ctermid.o ctermid.p \
	cuserid.o cuserid.p data.o data.p \
	doscan.o doscan.p fdopen.o fdopen.p \
	fgetc.o fgetc.p fgets.o fgets.p \
	filbuf.o filbuf.p findiop.o findiop.p \
	flsbuf.o flsbuf.p fopen.o fopen.p \
	fputc.o fputc.p fputs.o fputs.p \
	fread.o fread.p fseek.o fseek.p \
	ftell.o ftell.p fwrite.o fwrite.p \
	getchar.o getchar.p getpass.o getpass.p \
	gets.o gets.p getw.o getw.p \
	popen.o popen.p putchar.o putchar.p \
	puts.o puts.p putw.o putw.p \
	rew.o rew.p scanf.o scanf.p \
	setbuf.o setbuf.p tempnam.o tempnam.p \
	tmpfile.o tmpfile.p tmpnam.o tmpnam.p \
	ungetc.o ungetc.p: $(INC)/stdio.h
tempnam.o tempnam.p: $(INC)/string.h
fdopen.o fdopen.p \
	flsbuf.o flsbuf.p: $(INCSYS)/errno.h
getpass.o getpass.p: $(INC)/termio.h \
		$(INCSYS)/termio.h
doscan.o doscan.p: $(INC)/values.h
doscan.o doscan.p \
	scanf.o scanf.p: $(INC)/varargs.h
fgetc.o fgets.o filbuf.o findiop.o \
	flsbuf.o  fopen.o fputc.o fputs.o \
	fread.o fseek.o fwrite.o \
	getchar.o gets.o getw.o \
        putchar.o puts.o putw.o \
	setbuf.o ungetc.o: $(INCLIBC)/shlib.h

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c && mv $(*F).o $*.p

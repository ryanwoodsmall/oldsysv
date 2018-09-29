#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:gen/dial.mk	1.1"
#
# makefile for dial(3)
#
#

.SUFFIXES: .p
CC=cc
CFLAGS= -O -DSTANDALONE -DDIAL
PROF=
NONPROF=
ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include/sys
INCLIBC= ../../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC) -I$(UUDIR)
UUDIR=$(ROOT)/usr/src/cmd/uucp

DIALOBJ= callers.o conn.o getargs.o interface.o line.o stoa.o strecpy.o \
	strsave.o sysfiles.o ulockf.o

DIALPOBJ= callers.p conn.p getargs.p interface.p line.p stoa.p strecpy.p \
	strsave.p sysfiles.p ulockf.p

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c && mv $(*F).o $*.p

#	OBJECTS
dial.o: $(INC)/dial.h $(INCSYS)/termio.h $(INC)/errno.h $(INCSYS)/errno.h \
	$(INC)/fcntl.h $(INC)/signal.h $(INCSYS)/signal.h $(INC)/string.h \
	$(DIALOBJ) 
	$(NONPROF)@echo dial.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) \
		$(CFLAGS) -c dial.c
	$(NONPROF)mv dial.o dialsub.o
	$(NONPROF)$(LD) -r dialsub.o $(DIALOBJ) -o dial.o

callers.o:	$(UUDIR)/callers.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/callers.c

conn.o:		$(UUDIR)/conn.c
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/conn.c

getargs.o:	$(UUDIR)/getargs.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/getargs.c
 
interface.o:	$(UUDIR)/interface.c  $(UUDIR)/uucp.h
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/interface.c

line.o:	$(UUDIR)/line.c  $(UUDIR)/uucp.h
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/line.c

stoa.o:	$(UUDIR)/stoa.c
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/stoa.c

strecpy.o:	$(UUDIR)/strecpy.c
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/strecpy.c

strsave.o:	$(UUDIR)/strsave.c 
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/strsave.c

sysfiles.o:	$(UUDIR)/sysfiles.c  $(UUDIR)/sysfiles.h
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/sysfiles.c

ulockf.o:	$(UUDIR)/ulockf.c  $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(NONPROF)$(CC) -c $(CFLAGS)  $(UUDIR)/ulockf.c



#	POBJECTS
dial.p: $(INC)/dial.h $(INCSYS)/termio.h $(INC)/errno.h $(INCSYS)/errno.h \
	$(INC)/fcntl.h $(INC)/signal.h $(INCSYS)/signal.h $(INC)/string.h \
	$(DIALPOBJ) 
	$(PROF)@echo dial.c:
	$(PROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) \
		$(CFLAGS) -c -p dial.c
	$(PROF)mv dial.o dialsub.o
	$(PROF)$(LD) -r dialsub.o $(DIALPOBJ) -o dial.o
	$(PROF)mv dial.o dial.p

callers.p:	$(UUDIR)/callers.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/callers.c && mv callers.o callers.p

conn.p:		$(UUDIR)/conn.c
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/conn.c && mv conn.o conn.p

getargs.p:	$(UUDIR)/getargs.c   $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/getargs.c && mv getargs.o getargs.p
 
interface.p:	$(UUDIR)/interface.c  $(UUDIR)/uucp.h
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/interface.c && mv interface.o interface.p

line.p:	$(UUDIR)/line.c  $(UUDIR)/uucp.h
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/line.c && mv line.o line.p

stoa.p:	$(UUDIR)/stoa.c
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/stoa.c && mv stoa.o stoa.p

strecpy.p:	$(UUDIR)/strecpy.c
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/strecpy.c && mv strecpy.o strecpy.p

strsave.p:	$(UUDIR)/strsave.c 
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/strsave.c && mv strsave.o strsave.p

sysfiles.p:	$(UUDIR)/sysfiles.c  $(UUDIR)/sysfiles.h
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/sysfiles.c && mv sysfiles.o sysfiles.p

ulockf.p:	$(UUDIR)/ulockf.c  $(UUDIR)/uucp.h $(UUDIR)/parms.h
	$(PROF)$(CC) -c -p $(CFLAGS)  $(UUDIR)/ulockf.c && mv ulockf.o ulockf.p

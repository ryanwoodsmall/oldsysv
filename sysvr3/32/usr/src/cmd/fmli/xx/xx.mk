#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#
#ident	"@(#)vm.xx:src/xx/xx.mk	1.23"
#

INC=$(ROOT)/usr/include
USR=$(ROOT)/usr
USRLIB=$(ROOT)/usr/lib
CURSES_H =$(ROOT)/usr/include
LIBWISH	=	../wish/libwish.a
LIBVT	=	../vt/libvt.a
LIBFORM	=	../form/libform.a
LIBMENU	=	../menu/libmenu.a
LIBQUED	=	../qued/libqued.a
LIBOH	=	../oh/liboh.a
LIBOEU	=	../oeu/liboeu.a
LIBPROC	=	../proc/libproc.a
LIBSYS	=	../sys/libsys.a
LIBCURSES =	$(USRLIB)/libcurses.a

MKDIR	=	mkdir
BIN = $(USR)/bin
HEADER1=../inc
INCLUDE=	-I$(HEADER1)

LIBS = $(LIBWISH) $(LIBOH) $(LIBOEU) $(LIBFORM) $(LIBQUED) $(LIBMENU) $(LIBPROC) $(LIBVT) $(LIBSYS) $(LIBCURSES)
DIRS= $(USR)/vmsys $(USR)/oasys

CFLAGS= -O 

LDFLAGS =  -s

BCMDS =	fmli vsig 

CMDS = $(BCMDS) 

fmli: main.o $(LIBS)
	$(CC) $(LDFLAGS) -o $@ main.o $(LIBS) -lPW

main.o:	main.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h $(HEADER1)/ctl.h $(HEADER1)/vtdefs.h $(HEADER1)/status.h $(HEADER1)/moremacros.h

vsig: vsig.o
	$(CC) $(LDFLAGS) -o $@ vsig.o

vsig.o: vsig.c

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

###### Standard Makefile Targets ######

all:	$(CMDS)

install: all dirs $(BIN)
	@set +e;\
	for f in $(BCMDS);\
	do\
		install -f $(BIN) $$f;\
		$(CH) chgrp bin $(BIN)/$$f;\
		$(CH) chmod 755 $(BIN)/$$f;\
		$(CH) chown bin $(BIN)/$$f;\
	done

dirs:	$(DIRS)

$(DIRS):
	$(MKDIR) $@ 
	$(CH)chmod 755 $@

clean: 
	@set +e;\
	for f in $(BCMDS);\
	do\
		/bin/rm -f $$f;\
	done;\
	/bin/rm -f *.o

clobber:	clean

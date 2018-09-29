#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.vt:src/vt/vt.mk	1.13"
#

INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=libvt.a
CURSES_H=$(ROOT)/usr/include
HEADER1=../inc
INCLUDE=	-I$(CURSES_H) -I$(HEADER1)
CFLAGS= 	-O
AR=		ar

$(LIBRARY): \
		$(LIBRARY)(fits.o) \
		$(LIBRARY)(hide.o) \
		$(LIBRARY)(highlight.o) \
		$(LIBRARY)(indicator.o) \
		$(LIBRARY)(lp.o) \
		$(LIBRARY)(makebox.o) \
		$(LIBRARY)(message.o) \
		$(LIBRARY)(move.o) \
		$(LIBRARY)(offscreen.o) \
		$(LIBRARY)(physical.o) \
		$(LIBRARY)(redraw.o) \
		$(LIBRARY)(system.o) \
		$(LIBRARY)(vclose.o) \
		$(LIBRARY)(vcolor.o) \
		$(LIBRARY)(vcreate.o) \
		$(LIBRARY)(vctl.o) \
		$(LIBRARY)(vcurrent.o) \
		$(LIBRARY)(vdebug.o) \
		$(LIBRARY)(vflush.o) \
		$(LIBRARY)(vfork.o) \
		$(LIBRARY)(vinit.o) \
		$(LIBRARY)(vmark.o) \
		$(LIBRARY)(vreshape.o) \
		$(LIBRARY)(wclrwin.o) \
		$(LIBRARY)(wdelchar.o) \
		$(LIBRARY)(wgetchar.o) \
		$(LIBRARY)(wgo.o) \
		$(LIBRARY)(winschar.o) \
		$(LIBRARY)(wprintf.o) \
		$(LIBRARY)(wputchar.o) \
		$(LIBRARY)(wputs.o) \
		$(LIBRARY)(wreadchar.o) \
		$(LIBRARY)(wscrollwin.o) \
		$(LIBRARY)(showmail.o) \
		$(LIBRARY)(showdate.o) \
		$(LIBRARY)(status.o)  \
		$(LIBRARY)(working.o) 

$(LIBRARY)(vclose.o):	vclose.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/var_arrays.h

$(LIBRARY)(vcolor.o):	vcolor.c $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/color_pair.h $(HEADER1)/moremacros.h

$(LIBRARY)(vcreate.o):	vcreate.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/var_arrays.h $(HEADER1)/moremacros.h $(HEADER1)/color_pair.h

$(LIBRARY)(vctl.o):	vctl.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/ctl.h $(HEADER1)/vtdefs.h $(HEADER1)/attrs.h

$(LIBRARY)(vcurrent.o):	vcurrent.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(vdebug.o):	vdebug.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/var_arrays.h

$(LIBRARY)(fits.o):	fits.c $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(vflush.o):	vflush.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/attrs.h $(HEADER1)/color_pair.h

$(LIBRARY)(hide.o):	hide.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(highlight.o):	highlight.c $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/attrs.h

$(LIBRARY)(indicator.o):	indicator.c $(HEADER1)/vt.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(vinit.o):	vinit.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h $(HEADER1)/attrs.h $(HEADER1)/var_arrays.h $(HEADER1)/token.h

$(LIBRARY)(lp.o):	lp.c $(CURSES_H)/curses.h $(CURSES_H)/term.h

$(LIBRARY)(makebox.o):	makebox.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/attrs.h

$(LIBRARY)(move.o):	move.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(offscreen.o):	offscreen.c $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h

$(LIBRARY)(physical.o):	physical.c $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/token.h

$(LIBRARY)(redraw.o):	redraw.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(system.o):	system.c $(CURSES_H)/curses.h $(HEADER1)/wish.h

$(LIBRARY)(vfork.o):	vfork.c $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h

$(LIBRARY)(wclrwin.o):	wclrwin.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/attrs.h

$(LIBRARY)(wdelchar.o):	wdelchar.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(wgetchar.o):	wgetchar.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/token.h

$(LIBRARY)(wgo.o):	wgo.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(winschar.o):	winschar.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(wprintf.o):	wprintf.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h

$(LIBRARY)(wputchar.o):	wputchar.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(wputs.o):	wputs.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/attrs.h

$(LIBRARY)(wreadchar.o):	wreadchar.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(wscrollwin.o):	wscrollwin.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/attrs.h

$(LIBRARY)(vmark.o):	vmark.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(vreshape.o):	vreshape.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(vreshape.o):	vreshape.c $(HEADER1)/vt.h $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(showmail.o):	showmail.c $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h

$(LIBRARY)(showdate.o):	showdate.c $(HEADER1)/status.h $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h

$(LIBRARY)(working.o):	working.c $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/vt.h

$(LIBRARY)(status.o):	status.c $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h

$(LIBRARY)(message.o):	message.c $(CURSES_H)/curses.h $(CURSES_H)/term.h $(HEADER1)/wish.h $(HEADER1)/message.h $(HEADER1)/vtdefs.h $(HEADER1)/vt.h

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o


###### Standard makefile targets #######

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)

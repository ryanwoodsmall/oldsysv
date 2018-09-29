#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.wish:src/wish/wish.mk	1.19"
#


INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=libwish.a
CFLAGS= -O
AR=	ar
DEFS = -DRELEASE='"fmli R1.1 L9.6"'
HEADER1=../inc
INCLUDE= -I$(HEADER1) 
CFLAGS= -O

$(LIBRARY): \
		$(LIBRARY)(browse.o) \
		$(LIBRARY)(display.o) \
		$(LIBRARY)(error.o) \
		$(LIBRARY)(flush.o) \
		$(LIBRARY)(getstring.o) \
		$(LIBRARY)(global.o) \
		$(LIBRARY)(mudge.o) \
		$(LIBRARY)(objop.o) \
		$(LIBRARY)(stubs.o) \
		$(LIBRARY)(virtual.o) \
		$(LIBRARY)(wdwcreate.o) \
		$(LIBRARY)(wdwlist.o) \
		$(LIBRARY)(wdwmgmt.o)

$(LIBRARY)(browse.o):	browse.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h $(HEADER1)/ctl.h $(HEADER1)/moremacros.h

$(LIBRARY)(display.o):	display.c $(HEADER1)/wish.h  $(HEADER1)/typetab.h

$(LIBRARY)(error.o):	error.c $(HEADER1)/wish.h $(HEADER1)/token.h

$(LIBRARY)(flush.o):	flush.c $(HEADER1)/wish.h

$(LIBRARY)(getstring.o):	getstring.c $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/token.h $(HEADER1)/actrec.h $(HEADER1)/slk.h $(HEADER1)/winp.h $(HEADER1)/moremacros.h

$(LIBRARY)(global.o):	global.c  $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/ctl.h $(HEADER1)/terror.h $(HEADER1)/moremacros.h
	$(CC) -c $(CFLAGS) $(INCLUDE) $(DEFS) global.c
	$(AR) rv $@ global.o	 
	/bin/rm -f global.o

$(LIBRARY)(mudge.o):	mudge.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/vtdefs.h $(HEADER1)/actrec.h $(HEADER1)/slk.h $(HEADER1)/ctl.h $(HEADER1)/moremacros.h

$(LIBRARY)(objop.o):	objop.c $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/typetab.h $(HEADER1)/optabdefs.h $(HEADER1)/procdefs.h $(HEADER1)/moremacros.h

$(LIBRARY)(stubs.o):	stubs.c $(HEADER1)/wish.h $(HEADER1)/token.h

$(LIBRARY)(virtual.o):	virtual.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/vtdefs.h $(HEADER1)/actrec.h $(HEADER1)/slk.h $(HEADER1)/moremacros.h

$(LIBRARY)(wdwcreate.o):	wdwcreate.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h $(HEADER1)/ctl.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h

$(LIBRARY)(wdwlist.o):	wdwlist.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h $(HEADER1)/ctl.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h $(HEADER1)/moremacros.h

$(LIBRARY)(wdwmgmt.o):	wdwmgmt.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h $(HEADER1)/ctl.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h $(HEADER1)/moremacros.h

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

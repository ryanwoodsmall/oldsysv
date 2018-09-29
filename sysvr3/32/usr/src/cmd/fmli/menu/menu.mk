#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.menu:src/menu/menu.mk	1.5"
#


INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=libmenu.a
CFLAGS= -O
HEADER1=../inc
INCLUDE=	-I$(HEADER1)
AR=	ar



$(LIBRARY): \
		$(LIBRARY)(mclose.o) \
		$(LIBRARY)(mctl.o) \
		$(LIBRARY)(mcurrent.o) \
		$(LIBRARY)(mcustom.o) \
		$(LIBRARY)(mdefault.o) \
		$(LIBRARY)(mreshape.o) \
		$(LIBRARY)(stmenu.o)

$(LIBRARY)(mclose.o):	mclose.c ./menu.h $(HEADER1)/wish.h $(HEADER1)/var_arrays.h

$(LIBRARY)(mctl.o):	mctl.c ./menu.h $(HEADER1)/wish.h $(HEADER1)/ctl.h $(HEADER1)/menudefs.h

$(LIBRARY)(mcurrent.o):	mcurrent.c ./menu.h $(HEADER1)/wish.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h $(HEADER1)/attrs.h $(HEADER1)/ctl.h $(HEADER1)/color_pair.h

$(LIBRARY)(mcustom.o):	mcustom.c ./menu.h $(HEADER1)/wish.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h $(HEADER1)/var_arrays.h $(HEADER1)/ctl.h

$(LIBRARY)(mdefault.o):	mdefault.c ./menu.h $(HEADER1)/wish.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h $(HEADER1)/terror.h $(HEADER1)/ctl.h

$(LIBRARY)(mreshape.o):	mreshape.c ./menu.h $(HEADER1)/wish.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h $(HEADER1)/var_arrays.h $(HEADER1)/ctl.h

$(LIBRARY)(stmenu.o):	stmenu.c ./menu.h $(HEADER1)/wish.h $(HEADER1)/menudefs.h $(HEADER1)/vtdefs.h $(HEADER1)/token.h $(HEADER1)/ctl.h $(HEADER1)/moremacros.h $(HEADER1)/message.h

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

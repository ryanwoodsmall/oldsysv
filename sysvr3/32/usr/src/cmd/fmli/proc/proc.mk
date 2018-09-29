#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.proc:src/proc/proc.mk	1.4"
#

INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY = libproc.a
HEADER1=../inc
INCLUDE = -I$(HEADER1)
AR=		ar
CFLAGS= 	-O



$(LIBRARY):	\
		$(LIBRARY)(pclose.o) \
		$(LIBRARY)(pcurrent.o) \
		$(LIBRARY)(pctl.o) \
		$(LIBRARY)(pdefault.o) \
		$(LIBRARY)(list.o) \
		$(LIBRARY)(pnoncur.o) \
		$(LIBRARY)(open.o) \
		$(LIBRARY)(suspend.o)

$(LIBRARY)(pclose.o):	pclose.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h proc.h $(HEADER1)/procdefs.h

$(LIBRARY)(pcurrent.o):	pcurrent.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h proc.h $(HEADER1)/procdefs.h


$(LIBRARY)(pctl.o):	pctl.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h proc.h $(HEADER1)/procdefs.h $(HEADER1)/ctl.h

$(LIBRARY)(list.o):	list.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h proc.h $(HEADER1)/terror.h $(HEADER1)/ctl.h $(HEADER1)/menudefs.h $(HEADER1)/procdefs.h

$(LIBRARY)(pdefault.o):	pdefault.c $(HEADER1)/wish.h $(HEADER1)/terror.h proc.h $(HEADER1)/procdefs.h $(HEADER1)/actrec.h

$(LIBRARY)(open.o):	open.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/slk.h $(HEADER1)/actrec.h $(HEADER1)/terror.h $(HEADER1)/actrec.h $(HEADER1)/moremacros.h ./proc.h

$(LIBRARY)(pnoncur.o):	pnoncur.c $(HEADER1)/wish.h $(HEADER1)/terror.h proc.h $(HEADER1)/procdefs.h $(HEADER1)/actrec.h

$(LIBRARY)(suspend.o):	suspend.c $(HEADER1)/wish.h

.c.a:
	$(CC) -c $(CFLAGS)  $(INCLUDE) $<
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

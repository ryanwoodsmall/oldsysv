#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm:oeu/oeu.mk	1.1"
#

INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=liboeu.a
HEADER1=../inc
INCLUDE=	-I$(HEADER1)
DEFS=	-DJUSTCHECK

AR=		ar
CFLAGS=		-O


$(LIBRARY): \
		$(LIBRARY)(oeu.o) \
		$(LIBRARY)(oeucheck.o) \
		$(LIBRARY)(oeupack.o) \
		$(LIBRARY)(rm_atob.o) \
		$(LIBRARY)(genparse.o)

$(LIBRARY)(oeupack.o):	oeupack.c $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/io.h $(HEADER1)/tsys.h $(HEADER1)/io.h $(HEADER1)/typetab.h $(HEADER1)/obj.h $(HEADER1)/retcds.h $(HEADER1)/parse.h $(HEADER1)/smdef.h $(HEADER1)/mail.h $(HEADER1)/partabdefs.h $(HEADER1)/terror.h $(HEADER1)/mio.h $(HEADER1)/mess.h

$(LIBRARY)(oeucheck.o):	oeu.c $(HEADER1)/tsys.h $(HEADER1)/io.h $(HEADER1)/retcds.h $(HEADER1)/parse.h $(HEADER1)/smdef.h $(HEADER1)/typetab.h $(HEADER1)/mail.h $(HEADER1)/partabdefs.h $(HEADER1)/terror.h
	/bin/cp oeu.c oeucheck.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $(DEFS) oeucheck.c
	$(AR) rv $@ oeucheck.o
	/bin/rm -f oeucheck.o
	/bin/rm -f oeucheck.c

$(LIBRARY)(oeu.o):	oeu.c $(HEADER1)/tsys.h $(HEADER1)/io.h $(HEADER1)/retcds.h $(HEADER1)/parse.h $(HEADER1)/smdef.h $(HEADER1)/typetab.h $(HEADER1)/mail.h $(HEADER1)/partabdefs.h $(HEADER1)/terror.h

$(LIBRARY)(genparse.o):	genparse.c $(HEADER1)/tsys.h $(HEADER1)/io.h $(HEADER1)/retcds.h $(HEADER1)/parse.h $(HEADER1)/smdef.h $(HEADER1)/typetab.h $(HEADER1)/mail.h $(HEADER1)/terror.h $(HEADER1)/moremacros.h

$(LIBRARY)(rm_atob.o):	rm_atob.c

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

###### Standard makefile targets ######
all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)

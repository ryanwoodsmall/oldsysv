#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm:oh/oh.mk	1.1"
#


INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=liboh.a
HEADER1=../inc
INCLUDE= -I$(HEADER1)
DFLAGS=	-DWISH
CFLAGS=	-O
AR=	ar


$(LIBRARY): \
		$(LIBRARY)(externoot.o) \
		$(LIBRARY)(misc.o) \
		$(LIBRARY)(nextpart.o) \
		$(LIBRARY)(obj_to_par.o) \
		$(LIBRARY)(ootpart.o)  \
		$(LIBRARY)(partab.o)  \
		$(LIBRARY)(partabfunc.o)  \
		$(LIBRARY)(suffuncs.o)

$(LIBRARY)(externoot.o): 	externoot.c $(HEADER1)/tsys.h

$(LIBRARY)(misc.o): 	misc.c $(HEADER1)/tsys.h $(HEADER1)/moremacros.h

$(LIBRARY)(nextpart.o): nextpart.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h

$(LIBRARY)(obj_to_par.o): 	obj_to_par.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/partabdefs.h $(HEADER1)/optabdefs.h

$(LIBRARY)(ootpart.o):	ootpart.c $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h $(HEADER1)/mess.h

$(LIBRARY)(partab.o):	partab.c $(HEADER1)/tsys.h $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h 

$(LIBRARY)(partabfunc.o):	partabfunc.c $(HEADER1)/but.h $(HEADER1)/mio.h $(HEADER1)/tsys.h $(HEADER1)/typetab.h $(HEADER1)/ifuncdefs.h $(HEADER1)/optabdefs.h $(HEADER1)/partabdefs.h

$(LIBRARY)(suffuncs.o): suffuncs.c

.c.a:
	$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

###### Standard makefile targets #####

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)

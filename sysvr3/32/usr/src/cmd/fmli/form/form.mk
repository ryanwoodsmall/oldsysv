#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.form:src/form/form.mk	1.3"
#




INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=libform.a
HEADER1=../inc
INCLUDE=	-I$(HEADER1)
AR=		ar
CFLAGS=		-O


$(LIBRARY): \
		$(LIBRARY)(fcheck.o) \
		$(LIBRARY)(fclose.o) \
		$(LIBRARY)(fctl.o) \
		$(LIBRARY)(fcurrent.o) \
		$(LIBRARY)(fcustom.o) \
		$(LIBRARY)(fdefault.o) \
		$(LIBRARY)(frefresh.o)

$(LIBRARY)(fcheck.o):	fcheck.c $(HEADER1)/wish.h $(HEADER1)/ctl.h $(HEADER1)/token.h $(HEADER1)/winp.h $(HEADER1)/form.h $(HEADER1)/vtdefs.h

$(LIBRARY)(fclose.o):	fclose.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/winp.h $(HEADER1)/form.h $(HEADER1)/vtdefs.h $(HEADER1)/var_arrays.h

$(LIBRARY)(fctl.o):	fctl.c $(HEADER1)/wish.h $(HEADER1)/ctl.h $(HEADER1)/token.h $(HEADER1)/winp.h $(HEADER1)/form.h $(HEADER1)/vtdefs.h

$(LIBRARY)(fcurrent.o):	fcurrent.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/winp.h $(HEADER1)/form.h $(HEADER1)/vtdefs.h

$(LIBRARY)(fcustom.o):	fcustom.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/winp.h $(HEADER1)/form.h $(HEADER1)/var_arrays.h

$(LIBRARY)(fdefault.o):	fdefault.c $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/token.h $(HEADER1)/winp.h $(HEADER1)/form.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h

$(LIBRARY)(frefresh.o):	frefresh.c $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/winp.h $(HEADER1)/form.h $(HEADER1)/attrs.h

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

##### Standard makefile targets ######

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)

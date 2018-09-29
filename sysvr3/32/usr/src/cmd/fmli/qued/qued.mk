#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
# 	All Rights Reserved
#
#ident	"@(#)vm.qued:src/qued/qued.mk	1.9"
#

CURSES_H=$(ROOT)/usr/include
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=libqued.a
HEADER1=../inc
INCLUDE = -I$(CURSES_H) -I$(HEADER1)
CFLAGS = -O
AR=	ar


$(LIBRARY): \
		$(LIBRARY)(arrows.o) \
		$(LIBRARY)(copyfield.o) \
		$(LIBRARY)(fclear.o) \
		$(LIBRARY)(fgo.o) \
		$(LIBRARY)(fstream.o) \
		$(LIBRARY)(fput.o) \
		$(LIBRARY)(fread.o) \
		$(LIBRARY)(initfield.o) \
		$(LIBRARY)(editmulti.o) \
		$(LIBRARY)(editsingle.o) \
		$(LIBRARY)(getfield.o) \
		$(LIBRARY)(mfuncs.o) \
		$(LIBRARY)(multiline.o) \
		$(LIBRARY)(putfield.o) \
		$(LIBRARY)(setfield.o) \
		$(LIBRARY)(scrollbuf.o) \
		$(LIBRARY)(sfuncs.o) \
		$(LIBRARY)(singleline.o) \
		$(LIBRARY)(vfuncs.o) \
		$(LIBRARY)(wrap.o) 

$(LIBRARY)(arrows.o): arrows.c $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h $(HEADER1)/winp.h ./fmacs.h

$(LIBRARY)(copyfield.o): copyfield.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/moremacros.h

$(LIBRARY)(fclear.o): fclear.c $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/token.h

$(LIBRARY)(editmulti.o): editmulti.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h

$(LIBRARY)(editsingle.o): editsingle.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/terror.h $(HEADER1)/wish.h

$(LIBRARY)(fgo.o): fgo.c $(HEADER1)/winp.h ./fmacs.h

$(LIBRARY)(fstream.o): fstream.c $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h $(HEADER1)/attrs.h

$(LIBRARY)(fput.o): fput.c $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/attrs.h

$(LIBRARY)(fread.o): fread.c $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h $(HEADER1)/token.h $(HEADER1)/vt.h $(HEADER1)/vtdefs.h $(CURSES_H)/curses.h

$(LIBRARY)(getfield.o): getfield.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h $(HEADER1)/terror.h

$(LIBRARY)(initfield.o): initfield.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h $(HEADER1)/terror.h $(HEADER1)/vtdefs.h $(HEADER1)/ctl.h $(HEADER1)/attrs.h

$(LIBRARY)(mfuncs.o): mfuncs.c $(HEADER1)/wish.h $(HEADER1)/ctl.h $(HEADER1)/winp.h $(HEADER1)/vt.h $(CURSES_H)/curses.h ./fmacs.h

$(LIBRARY)(multiline.o): multiline.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h

$(LIBRARY)(putfield.o): putfield.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h $(HEADER1)/moremacros.h $(HEADER1)/terror.h

$(LIBRARY)(setfield.o): setfield.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/terror.h

$(LIBRARY)(scrollbuf.o): scrollbuf.c $(HEADER1)/wish.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/terror.h $(HEADER1)/token.h

$(LIBRARY)(sfuncs.o): sfuncs.c $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/token.h $(HEADER1)/ctl.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/attrs.h

$(LIBRARY)(singleline.o): singleline.c $(HEADER1)/token.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/wish.h

$(LIBRARY)(vfuncs.o): vfuncs.c $(HEADER1)/wish.h $(HEADER1)/vtdefs.h $(HEADER1)/token.h $(HEADER1)/ctl.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/terror.h

$(LIBRARY)(wrap.o): wrap.c $(HEADER1)/wish.h $(HEADER1)/winp.h ./fmacs.h $(HEADER1)/terror.h

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o


###### Standard makefile targets #####

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	libqued.a 

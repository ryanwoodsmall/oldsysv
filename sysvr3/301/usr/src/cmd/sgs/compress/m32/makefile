#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)compress:m32/makefile	1.19"
#	COMPRESS MAKEFILE
#
#
SGS=m32
OWN = bin
GRP = bin
ROOT =
BASE = ../..
#
SGS=m32
INCCOM = $(BASE)/inc/common
COMDIR = $(BASE)/compress/common
INCLST = -I$(COMDIR) -I$(INCCOM)
BINDIR = $(BASE)/xenv/m32/bin
LIBLD	= $(BASE)/libld/m32/libld.a
#
STRIP=strip
SPFLAGS=
STRIP_CMD=$(STRIP) $(SPFLAGS)
#
LINT = lint
LINTFLAGS=-p
LINT_CMD=$(LINT) $(LINTFLAGS)
#
CC = cc
CFLG = -O -c
OBJECTS = pass1.o pass2.o
LOADS = $(OBJECTS) $(LIBLD)
LDLIBS=
#
#
all:	cprs
#
cprs:  $(LOADS)
	$(CC) $(LOADS) -o  cprs  $(LDLIBS)
#
pass1.o:  $(INCCOM)/filehdr.h $(INCCOM)/storclass.h $(INCCOM)/syms.h \
	$(INCCOM)/ldfcn.h $(COMDIR)/tagitem.h $(COMDIR)/pass1.c
	$(CC) $(CFLG) $(INCLST) $(COMDIR)/pass1.c
#
pass2.o:  $(INCCOM)/filehdr.h $(INCCOM)/syms.h \
	$(INCCOM)/scnhdr.h $(INCCOM)/linenum.h $(INCCOM)/reloc.h \
	$(INCCOM)/ldfcn.h $(COMDIR)/tagitem.h  $(COMDIR)/pass2.c
	$(CC) $(CFLG) $(INCLST) $(COMDIR)/pass2.c
#
save:	$(BINDIR)/$(SGS)cprs
	-rm -f $(BINDIR)/$(SGS)cprs.back
	cp $(BINDIR)/$(SGS)cprs $(BINDIR)/$(SGS)cprs.back
#
install:	$(BINDIR)/$(SGS)cprs
#
$(BINDIR)/$(SGS)cprs:	cprs
	cp cprs cprs.bak
	-$(STRIP_CMD) cprs
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)cprs cprs
	mv cprs.bak cprs
#
uninstall:	$(BINDIR)/$(SGS)cprs.back
	-rm -f $(BINDIR)/$(SGS)cprs
	cp $(BINDIR)/$(SGS)cprs.back $(BINDIR)/$(SGS)cprs
#
shrink:
	-rm -f $(OBJECTS) cprs
#
lint:	$(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCCOM)/linenum.h\
	$(INCCOM)/reloc.h $(INCCOM)/scnhdr.h $(INCCOM)/storclass.h\
	$(INCCOM)/syms.h $(COMDIR)/tagitem.h $(COMDIR)/pass1.c $(COMDIR)/pass2.c
	$(LINT_CMD) -I$(INCCOM) -I$(COMDIR) $(COMDIR)/*.c

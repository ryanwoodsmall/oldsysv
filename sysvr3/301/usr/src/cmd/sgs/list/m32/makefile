#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)list:m32/makefile	1.14"
#
#	BELLMAC-32 LIST MAKEFILE
#
#
SGS=m32
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O -c
ARCH=AR32W
#	Set FLEX to -DFLEXNAMES for flexnames
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) $(FLEX)
INCLIST=-I$(M32INC) -I$(COMINC) -I$(COMLIST) -I$(USRINC)
ENV=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
LDFLAGS=-i $(FFLAG)
LD_CMD=$(CC) $(LDFLAGS)
#
LDLIBS=
#
STRIP=strip
SPFLAGS=
STRIP_CMD=$(STRIP) $(SPFLAGS)
#
LINT=lint
LINTFLAGS=-p
LINT_CMD=$(LINT) $(LINTFLAGS)
#
#	Internal Directories
#
BASE= ../..
BINDIR= $(BASE)/xenv/m32/bin
LIBDIR= $(BASE)/xenv/m32/lib
M32INC= $(BASE)/inc/m32
COMLIST= $(BASE)/list/common
COMINC= $(BASE)/inc/common
#
#	External Directories
#
ROOT=
VERSION=
USRINC=$(ROOT)/usr/$(VERSION)include
#
LIBLD	= $(BASE)/libld/m32/libld.a
OBJECTS= main.o initfcns.o process.o symfcns.o findname.o parsefcns.o \
	srciofcns.o fcns.o
#
all:	list
#
list:		$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o list $(OBJECTS) $(LIBLD) $(LDLIBS)
#
main.o:	$(COMLIST)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
	$(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/main.c
#
initfcns.o:	$(COMLIST)/initfcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
		$(M32INC)/paths.h $(M32INC)/sgs.h $(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/initfcns.c
#
process.o:	$(COMLIST)/process.c $(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/process.c
#
symfcns.o:	$(COMLIST)/symfcns.c $(COMINC)/filehdr.h $(COMINC)/syms.h $(COMINC)/ldfcn.h \
		$(COMINC)/storclass.h $(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/symfcns.c
#
findname.o:	$(COMLIST)/findname.c $(COMINC)/syms.h $(COMINC)/storclass.h \
		$(COMLIST)/defs.h $(COMLIST)/findname.h
	$(CC_CMD) $(COMLIST)/findname.c
#
parsefcns.o:	$(COMLIST)/parsefcns.c $(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/parsefcns.c
#
srciofcns.o:	$(COMLIST)/srciofcns.c $(COMINC)/filehdr.h $(COMINC)/linenum.h \
		$(COMINC)/ldfcn.h $(COMLIST)/defs.h $(COMINC)/sgsmacros.h
	$(CC_CMD) $(COMLIST)/srciofcns.c
#
fcns.o:	$(COMLIST)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
	$(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/fcns.c
#
#
shrink:
	-rm -f $(OBJECTS) list
#
save:	$(BINDIR)/$(SGS)list
	-rm -f $(BINDIR)/$(SGS)list.back
	cp $(BINDIR)/$(SGS)list $(BINDIR)/$(SGS)list.back
#
install:	$(BINDIR)/$(SGS)list
#
$(BINDIR)/$(SGS)list:	list
	cp list list.bak
	-$(STRIP_CMD) list
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)list list
	mv list.bak list
#
uninstall:	$(BINDIR)/$(SGS)list.back
	-rm -f $(BINDIR)/$(SGS)list
	cp $(BINDIR)/$(SGS)list.back $(BINDIR)/$(SGS)list
#
lint:	$(COMLIST)/main.c $(COMLIST)/initfcns.c $(COMLIST)/process.c \
	$(COMLIST)/symfcns.c $(COMLIST)/findname.c $(COMLIST)/parsefcns.c \
	$(COMLIST)/srciofcns.c $(COMLIST)/fcns.c \
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
	$(COMINC)/linenum.h $(COMINC)/syms.h $(COMINC)/storclass.h \
	$(COMLIST)/defs.h $(COMLIST)/findname.h \
	$(M32INC)/paths.h
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMLIST)/*.c

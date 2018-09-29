#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dump:m32/makefile	1.14"
#
#	BELLMAC-32 DUMP MAKEFILE
#
#
SGS=m32
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O -c
ARCH=AR32WR
#	Set ARFORMAT to "PORTAR" for portable archive format (post 5.0)
ARFORMAT=PORTAR
#	Set FLEX to -DFLEXNAMES for flexnames (post 6.0)
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) -D$(ARFORMAT) $(FLEX)
INCLIST=-I$(COMINC) -I$(COMDUMP) -I$(M32INC) -I$(USRINC)
ENV=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
FFLAG=
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
#	External Directories
ROOT=
VERSION=
USRINC= $(ROOT)/usr/$(VERSION)include
#
#
#	Internal Directories
BASE= ../..
BINDIR= $(BASE)/xenv/m32/bin
COMDUMP= $(BASE)/dump/common
COMINC= $(BASE)/inc/common
M32INC= $(BASE)/inc/m32
#
LIBLD	= $(BASE)/libld/m32/libld.a
#
OBJECTS= main.o setflags.o openobject.o process.o fcns.o
#
all:	dump
#
dump:	$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o dump $(OBJECTS) $(LIBLD) $(LDLIBS)
#
main.o:	$(COMDUMP)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMDUMP)/filedefs.h \
	 $(COMDUMP)/dumpmap.h $(M32INC)/sgs.h
	$(CC_CMD) $(COMDUMP)/main.c
#
setflags.o:	$(COMDUMP)/setflags.c $(COMDUMP)/flagdefs.h $(M32INC)/sgs.h
	$(CC_CMD) $(COMDUMP)/setflags.c
#
openobject.o:	$(COMDUMP)/openobject.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	       $(M32INC)/sgs.h $(COMDUMP)/filedefs.h
	$(CC_CMD) $(COMDUMP)/openobject.c
#
process.o:	$(COMDUMP)/process.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h\
	    $(COMDUMP)/filedefs.h $(COMDUMP)/process.h $(COMDUMP)/flagdefs.h
	$(CC_CMD) $(COMDUMP)/process.c
#
fcns.o:	$(COMDUMP)/fcns.c $(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/reloc.h \
	 $(COMINC)/linenum.h $(COMINC)/syms.h $(COMINC)/storclass.h $(COMINC)/ldfcn.h \
	 $(M32INC)/sgs.h
	$(CC_CMD) $(COMDUMP)/fcns.c
#
save:	$(BINDIR)/$(SGS)dump
	-rm -f $(BINDIR)/$(SGS)dump.back
	cp $(BINDIR)/$(SGS)dump $(BINDIR)/$(SGS)dump.back
#
install:	$(BINDIR)/$(SGS)dump
#
$(BINDIR)/$(SGS)dump:	dump
	cp dump dump.bak
	-$(STRIP_CMD) dump
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)dump dump
	mv dump.bak dump
#
uninstall:	$(BINDIR)/$(SGS)dump.back
	-rm -f $(BINDIR)/$(SGS)dump
	cp $(BINDIR)/$(SGS)dump.back $(BINDIR)/$(SGS)dump
#
lint:	$(COMDUMP)/main.c $(COMDUMP)/setflags.c $(COMDUMP)/openobject.c \
	$(COMDUMP)/process.c $(COMDUMP)/fcns.c \
	$(COMDUMP)/dumpmap.h \
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMDUMP)/filedefs.h \
	$(M32INC)/sgs.h $(COMDUMP)/flagdefs.h $(M32INC)/paths.h \
	$(COMINC)/syms.h $(COMINC)/storclass.h $(COMINC)/linenum.h \
	$(COMINC)/scnhdr.h $(COMINC)/reloc.h
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMDUMP)/*.c
#
shrink:
	-rm -f $(OBJECTS) dump

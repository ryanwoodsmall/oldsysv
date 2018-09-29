#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)chkshlib:m32/makefile	1.2"
#
#	3B2/3B5 chkshlib MAKEFILE
#
#
SGS=m32
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
DEFLIST=
INCLIST=-I$(M32INC) -I$(COMINC) -I$(COMDIR) -I$(USRINC)
ENV=
CC_CMD=$(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
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
#
ROOT=
VERSION=
USRINC= $(ROOT)/usr/$(VERSION)include
#
#	Internal Directories
#
BASE= ../..
BINDIR= $(BASE)/xenv/m32/bin
LIBDIR= $(BASE)/xenv/m32/lib
M32INC= $(BASE)/inc/m32
COMDIR= $(BASE)/chkshlib/common
M32DIR= $(BASE)/chkshlib/m32
COMINC= $(BASE)/inc/common
#
LIBLD	= $(BASE)/libld/m32/libld.a
OBJECTS = check.o fcns.o main.o read.o

#
all:	chkshlib
#
chkshlib:	$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o chkshlib $(OBJECTS) $(LIBLD) $(LDLIBS)
#
main.o:	$(COMDIR)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
	$(COMINC)/aouthdr.h $(COMINC)/reloc.h $(COMINC)/syms.h $(COMINC)/scnhdr.h \
	$(COMDIR)/chkshlib.h
	$(CC_CMD) $(COMDIR)/main.c
#
check.o: $(COMDIR)/check.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/reloc.h $(COMINC)/syms.h $(COMINC)/scnhdr.h $(COMDIR)/chkshlib.h 
	$(CC_CMD) $(COMDIR)/check.c
#
fcns.o:	$(COMDIR)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/reloc.h $(COMINC)/syms.h $(COMINC)/scnhdr.h $(COMDIR)/chkshlib.h 
	$(CC_CMD) $(COMDIR)/fcns.c
#
read.o: $(COMDIR)/read.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/reloc.h $(COMINC)/syms.h $(COMINC)/scnhdr.h $(COMDIR)/chkshlib.h 
	$(CC_CMD) $(COMDIR)/read.c
#
save:	$(BINDIR)/$(SGS)chkshlib
	-rm -f $(BINDIR)/$(SGS)chkshlib.back
	cp $(BINDIR)/$(SGS)chkshlib $(BINDIR)/$(SGS)chkshlib.back
#
install:	$(BINDIR)/$(SGS)chkshlib
#
$(BINDIR)/$(SGS)chkshlib: chkshlib
	cp chkshlib chkshlib.bak
	-$(STRIP_CMD) chkshlib
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)chkshlib chkshlib
	mv chkshlib.bak chkshlib
#
uninstall:	$(BINDIR)/$(SGS)chkshlib.back
	-rm -f $(BINDIR)/$(SGS)chkshlib
	cp $(BINDIR)/$(SGS)chkshlib.back $(BINDIR)/$(SGS)chkshlib
#
shrink:
	-rm -f $(OBJECTS) chkshlib
#
lint:	$(COMDIR)/check.c $(COMDIR)/fcns.c $(COMDIR)/chkshlib.h \
	$(COMDIR)/main.c $(COMDIR)/read.c \
	$(COMINC)/aouthdr.h $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/syms.h \
	$(COMINC)/reloc.h $(M32INC)/paths.h $(M32INC)/sgs.h \
	$(COMINC)/scnhdr.h
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMDIR)/*.c

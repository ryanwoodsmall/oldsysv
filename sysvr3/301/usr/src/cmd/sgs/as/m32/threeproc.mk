#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)as:m32/threeproc.mk	1.17"
#
#	BELLMAC-32 AS MAKEFILE
#
#
SGS=m32
CC=cc
CFLAGS=-O -c 
ARCH=AR16WR
DBO=FBO
FIX=NOFIX
#FIX=CHIPFIX
PCCALL=CALLPCREL
FLEX=-DFLEXNAMES
DEFLIST=-D$(ARCH) -D$(DBO) -D$(FIX) -D$(PCCALL) $(FLEX)
INCLIST=-I. -I$(COMAS) -I$(COMINC) -I$(M32INC) -I$(USRINC)
ENV=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
B30=
#
LDFLAGS=-i $(FFLAG)
LDLIBS=
LD_CMD=$(CC) $(LDFLAGS)
#
YACC=yacc
YFLAGS=-d
YACC_CMD=$(YACC) $(YFLAGS)
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
COMAS= $(BASE)/as/common
COMINC= $(BASE)/inc/common
#
#	External Directories
#
ROOT=
USRINC=$(ROOT)/usr/include
#
LIBM=-lm
#
OFILES0=pass0.o
OFILES1=parse.o code.o errors.o gencode.o instab.o \
	pass1.o symbols1.o expand1.o expand2.o float.o
OFILES2=addr1.o addr2.o codeout.o getstab.o pass2.o obj.o symlist.o \
	symbols2.o errors.o
#
CFILES0=$(COMAS)/pass0.c
CFILES1=parse.c $(COMAS)/code.c $(COMAS)/errors.c gencode.c $(COMAS)/instab.c \
	$(COMAS)/pass1.c $(COMAS)/symbols1.c \
	$(COMAS)/expand1.c expand2.c float.c
CFILES2=$(COMAS)/addr1.c addr2.c $(COMAS)/codeout.c $(COMAS)/getstab.c \
	$(COMAS)/pass2.c $(COMAS)/obj.c $(COMAS)/symbols2.c \
	$(COMAS)/symlist.c $(COMAS)/errors.c
#
HFILES1=$(COMAS)/codeout.h gendefs.h instab.h ops.out \
	$(COMAS)/symbols.h symbols2.h $(M32INC)/paths.h \
	$(COMAS)/expand.h expand2.h parse.h systems.h
HFILES2=$(COMAS)/codeout.h gendefs.h $(COMAS)/symbols.h symbols2.h \
	$(COMINC)/reloc.h $(COMINC)/syms.h $(COMINC)/storclass.h $(COMINC)/filehdr.h \
	$(COMINC)/scnhdr.h $(COMINC)/linenum.h instab.h $(M32INC)/sgs.h systems.h
#
all		: as as1 as2
#
#	PASS 0
as	:	 $(OFILES0)
		$(LD_CMD) -o as $(OFILES0) $(LDLIBS)
#
pass0.o	: $(COMAS)/pass0.c pass0.h $(M32INC)/paths.h $(M32INC)/sgs.h systems.h
		$(CC_CMD) $(COMAS)/pass0.c
#
#	PASS 1
as1	:	 $(OFILES1)
		$(LD_CMD) -o as1 $(OFILES1) $(LIBM) $(LDLIBS)
#
parse.o		: parse.c $(COMAS)/symbols.h instab.h gendefs.h systems.h
		$(CC_CMD) $(B30) parse.c
#
parse.c parse.h	: parse.y
		$(YACC_CMD) parse.y
		mv y.tab.c parse.c
		mv y.tab.h parse.h
#
code.o		: $(COMAS)/code.c $(COMAS)/symbols.h $(COMAS)/codeout.h \
		  gendefs.h systems.h
		$(CC_CMD) $(COMAS)/code.c
#
errors.o	: $(COMAS)/errors.c gendefs.h systems.h
		$(CC_CMD) $(COMAS)/errors.c
#
pass1.o		: $(COMAS)/pass1.c $(M32INC)/paths.h $(COMAS)/symbols.h \
		  gendefs.h systems.h
		$(CC_CMD) $(COMAS)/pass1.c
#
symbols1.o	: $(COMAS)/symbols1.c $(COMAS)/symbols.c $(COMAS)/symbols.h \
		  symbols2.h systems.h
		$(CC_CMD) $(COMAS)/symbols1.c
#
instab.o	: $(COMAS)/instab.c instab.h ops.out $(COMAS)/symbols.h \
		  parse.h systems.h
		$(CC_CMD) $(COMAS)/instab.c
#
gencode.o	: gencode.c $(COMAS)/symbols.h parse.h instab.h gendefs.h \
		  $(COMAS)/expand.h expand2.h systems.h
		$(CC_CMD) gencode.c
#
expand1.o	: $(COMAS)/expand1.c $(COMAS)/expand.h $(COMAS)/symbols.h \
		  gendefs.h systems.h
		$(CC_CMD) $(COMAS)/expand1.c
#
expand2.o	: expand2.c $(COMAS)/expand.h expand2.h $(COMAS)/symbols.h systems.h
		$(CC_CMD) expand2.c
#
float.o		: float.c instab.h $(COMAS)/symbols.h systems.h
		$(CC_CMD) float.c
#
lint1.out	: $(CFILES1) $(HFILES1) $(COMAS)/symbols.c
		$(LINT_CMD) $(DEFLIST) $(INCLIST) $(CFILES1) >lint1.out
		wc -l lint1.out
#
#	PASS 2
as2	:	 $(OFILES2)
		$(LD_CMD) -o as2 $(OFILES2) $(LDLIBS)
#
addr1.o		: $(COMAS)/addr1.c $(COMINC)/reloc.h $(COMINC)/syms.h \
		  $(COMINC)/storclass.h $(COMINC)/linenum.h $(COMINC)/filehdr.h \
		  gendefs.h $(COMAS)/symbols.h $(COMAS)/codeout.h systems.h
		$(CC_CMD) $(COMAS)/addr1.c
#
addr2.o		: addr2.c $(COMINC)/reloc.h $(COMINC)/storclass.h \
		  $(COMINC)/syms.h gendefs.h $(COMAS)/symbols.h \
		  $(COMAS)/codeout.h instab.h systems.h
		$(CC_CMD) addr2.c
#
codeout.o	: $(COMAS)/codeout.c $(COMAS)/symbols.h $(COMAS)/codeout.h \
		  gendefs.h systems.h
		$(CC_CMD) $(COMAS)/codeout.c
#
getstab.o	: $(COMAS)/getstab.c gendefs.h $(COMAS)/symbols.h systems.h
		$(CC_CMD) $(COMAS)/getstab.c
#
pass2.o		: $(COMAS)/pass2.c gendefs.h $(COMAS)/symbols.h systems.h
		$(CC_CMD) $(COMAS)/pass2.c
#
obj.o		: $(COMAS)/obj.c $(COMINC)/filehdr.h $(COMINC)/linenum.h instab.h \
		  $(COMINC)/reloc.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		  $(COMINC)/storclass.h $(COMAS)/symbols.h $(COMAS)/codeout.h \
		  gendefs.h $(M32INC)/sgs.h systems.h
		$(CC_CMD) $(COMAS)/obj.c
#
symbols2.o	: $(COMAS)/symbols2.c $(COMAS)/symbols.c $(COMAS)/symbols.h \
		  symbols2.h systems.h
		$(CC_CMD) $(COMAS)/symbols2.c
#
symlist.o	: $(COMAS)/symlist.c $(COMAS)/symbols.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h gendefs.h systems.h
		$(CC_CMD) $(COMAS)/symlist.c
#
lint2.out	: $(CFILES2) $(HFILES2) $(COMAS)/symbols.c
		$(LINT_CMD) $(DEFLIST) $(INCLIST) $(CFILES2) >lint2.out
		wc -l lint2.out
#
save		: $(BINDIR)/$(SGS)as $(LIBDIR)/$(SGS)as1 $(LIBDIR)/$(SGS)as2 \
		  $(LIBDIR)/cm4defs $(LIBDIR)/cm4tvdefs
		-rm -f $(BINDIR)/$(SGS)as.back
		cp $(BINDIR)/$(SGS)as $(BINDIR)/$(SGS)as.back
		-rm -f $(LIBDIR)/$(SGS)as1.back $(LIBDIR)/$(SGS)as2.back
		cp $(LIBDIR)/$(SGS)as1 $(LIBDIR)/$(SGS)as1.back
		cp $(LIBDIR)/$(SGS)as2 $(LIBDIR)/$(SGS)as2.back
		-rm -f $(LIBDIR)/cm4defs.back $(LIBDIR)/cm4tvdefs.back
		cp $(LIBDIR)/cm4defs $(LIBDIR)/cm4defs.back
		cp $(LIBDIR)/cm4tvdefs $(LIBDIR)/cm4tvdefs.back
#
install:	 $(BINDIR)/$(SGS)as \
		 $(LIBDIR)/$(SGS)as1 $(LIBDIR)/$(SGS)as2 \
		 $(LIBDIR)/cm4defs $(LIBDIR)/cm4tvdefs
#
$(BINDIR)/$(SGS)as:	as
		-rm -f $(BINDIR)/$(SGS)as
		cp as $(BINDIR)/$(SGS)as
		-$(STRIP_CMD) $(BINDIR)/$(SGS)as
$(LIBDIR)/$(SGS)as1:	as1
		-rm -f $(LIBDIR)/$(SGS)as1
		cp as1 $(LIBDIR)/$(SGS)as1
		-$(STRIP_CMD) $(LIBDIR)/$(SGS)as1
$(LIBDIR)/$(SGS)as2:	as2
		-rm -f $(LIBDIR)/$(SGS)as2
		cp as2 $(LIBDIR)/$(SGS)as2
		-$(STRIP_CMD) $(LIBDIR)/$(SGS)as2
$(LIBDIR)/cm4defs:	cm4defs
		-rm -f $(LIBDIR)/cm4defs
		cp cm4defs $(LIBDIR)/cm4defs
$(LIBDIR)/cm4tvdefs:	cm4tvdefs
		-rm -f $(LIBDIR)/cm4tvdefs
		cp cm4tvdefs $(LIBDIR)/cm4tvdefs
#
uninstall	: $(BINDIR)/$(SGS)as.back \
		  $(LIBDIR)/$(SGS)as1.back $(LIBDIR)/$(SGS)as2.back \
		  $(LIBDIR)/cm4defs.back $(LIBDIR)/cm4tvdefs.back
		-rm -f $(BINDIR)/$(SGS)as
		cp $(BINDIR)/$(SGS)as.back $(BINDIR)/$(SGS)as
		-rm -f $(LIBDIR)/$(SGS)as1 $(LIBDIR)/$(SGS)as2
		cp $(LIBDIR)/$(SGS)as1.back $(LIBDIR)/$(SGS)as1
		cp $(LIBDIR)/$(SGS)as2.back $(LIBDIR)/$(SGS)as2
		-rm -f $(LIBDIR)/cm4defs $(LIBDIR)/cm4tvdefs
		cp $(LIBDIR)/cm4defs.back $(LIBDIR)/cm4defs
		cp $(LIBDIR)/cm4tvdefs.back $(LIBDIR)/cm4tvdefs
#
shrink		:
		-rm -f $(OFILES0) $(OFILES1) $(OFILES2) parse.c parse.h y.output lint?.out
		-rm -f as as1 as2

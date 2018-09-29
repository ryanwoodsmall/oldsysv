#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)as:m32/oneproc.mk	1.27"
#
#	WE32000 AS MAKEFILE
#

SGS=m32
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O -c
ARCH=AR32W
DBO=FBO
PCCALL=CALLPCREL
#	Set FLEX to -DFLEXNAMES for flexnames (post 6.0)
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) -D$(DBO) -DONEPROC -D$(PCCALL) $(FLEX) -D$(MAC) -DM32
INCLIST=-I. -I$(COMAS) -I$(3BAS) -I$(COMINC) -I$(M32INC) -I$(USRINC)
ENV=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
LDFLAGS=$(FFLAG)
LDLIBS=
LD_CMD=$(CC) $(LDFLAGS)
#
B30=
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
M32INC= $(BASE)/inc/m32
COMAS= ../common
3BAS=../3b
COMINC= $(BASE)/inc/common
BINDIR= $(BASE)/xenv/m32/bin
LIBDIR= $(BASE)/xenv/m32/lib
#
#	External Directories
#
ROOT=
VERSION=
USRINC=$(ROOT)/usr/$(VERSION)include
#
LIBM=-lm
#
OFILES=as_ctype.o pass0.o instab.o program.o operand.o gencode.o pass1.o symbols.o \
	code.o expand1.o expand2.o errors.o addr2.o float.o \
	pass2.o addr1.o codeout.o getstab.o obj.o symlist.o swagen.o \
	maugen.o unused.o 
#
CFILES=$(3BAS)/as_ctype.c $(COMAS)/pass0.c \
	operand.c program.c $(COMAS)/code.c $(COMAS)/errors.c gencode.c \
	swagen.c $(COMAS)/instab.c \
	$(COMAS)/pass1.c $(COMAS)/symbols.c \
	expand1.c expand2.c float.c \
	$(COMAS)/addr1.c addr2.c $(COMAS)/codeout.c \
	$(COMAS)/getstab.c $(COMAS)/pass2.c $(COMAS)/obj.c \
	$(COMAS)/symlist.c $(COMAS)/unused.c maugen.c
#
HFILES=$(3BAS)/as_ctype.h $(COMAS)/codeout.h gendefs.h instab.h \
	ops.out $(COMAS)/section.h $(COMAS)/symbols.h $(COMAS)/temppack.h \
	$(M32INC)/paths.h $(COMAS)/expand.h expand2.h program.h \
	$(COMINC)/reloc.h $(COMINC)/syms.h $(COMINC)/storclass.h\
	$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/linenum.h \
	$(M32INC)/sgs.h
#
all	: as
#
as	: $(OFILES)
		$(LD_CMD) -o as $(OFILES) $(LIBM) $(LDLIBS)
#
as_ctype.o	: $(3BAS)/as_ctype.h $(3BAS)/as_ctype.c
		$(CC_CMD) $(3BAS)/as_ctype.c
#
pass0.o		: $(COMAS)/pass0.c $(M32INC)/paths.h \
		$(M32INC)/sgs.h gendefs.h systems.h
		$(CC_CMD) $(COMAS)/pass0.c
#
program.o	: $(3BAS)/as_ctype.h program.c $(COMAS)/symbols.h \
		$(COMINC)/filehdr.h program.h instab.h \
		systems.h ind.out typ.out mnm.out sem.out gendefs.h \
		$(COMINC)/scnhdr.h $(COMINC)/storclass.h $(M32INC)/sgs.h \
		$(COMINC)/syms.h
		$(CC_CMD) $(B30) -DYYDEBUG=0 program.c
#
make_tables : $(3BAS)/make_tables.c
		cc -o make_tables $(3BAS)/make_tables.c
#
ind.out typ.out mnm.out sem.out : ops.in make_tables
		./make_tables <ops.in
# instab.c is really no longer needed (predefineds are kept in program.c and operand.c)
# we link it in anyway because the symbol tables stuff in common references it
ops.out	:
	  echo >ops.out

parse.h :
	  echo >parse.h
#
operand.o: operand.c $(3BAS)/as_ctype.h program.h $(COMAS)/symbols.h instab.h\
	   systems.h
		$(CC_CMD) operand.c

#
code.o		: $(COMAS)/code.c $(COMAS)/symbols.h $(COMAS)/codeout.h \
		  $(COMINC)/scnhdr.h \
		  $(COMAS)/section.h gendefs.h systems.h $(COMAS)/temppack.h
		$(CC_CMD) $(COMAS)/code.c
#
errors.o	: $(COMAS)/errors.c gendefs.h systems.h
		$(CC_CMD) $(COMAS)/errors.c
#
pass1.o		: $(COMAS)/pass1.c $(COMAS)/codeout.h $(COMAS)/section.h \
		  $(M32INC)/paths.h $(COMAS)/symbols.h gendefs.h systems.h
		$(CC_CMD) $(COMAS)/pass1.c
#
instab.o	: $(COMAS)/instab.c instab.h ops.out \
		  $(COMAS)/symbols.h parse.h systems.h
		$(CC_CMD) $(COMAS)/instab.c
#
gencode.o	: gencode.c $(COMAS)/symbols.h instab.h systems.h \
		  program.h gendefs.h $(COMAS)/expand.h expand2.h
		$(CC_CMD) gencode.c
#
swagen.o	: swagen.c $(COMAS)/symbols.h instab.h systems.h \
		  gendefs.h $(COMAS)/expand.h expand2.h program.h
		$(CC_CMD) swagen.c
#
maugen.o	: maugen.c maugen.h $(COMAS)/symbols.h instab.h systems.h \
		  program.h gendefs.h $(COMAS)/expand.h expand2.h \
		  $(COMINC)/scnhdr.h
		$(CC_CMD) maugen.c
#
expand1.o	: expand1.c $(COMAS)/expand.h $(COMAS)/symbols.h gendefs.h systems.h
		$(CC_CMD) expand1.c
#
expand2.o	: expand2.c $(COMAS)/expand.h expand2.h $(COMAS)/symbols.h systems.h
		$(CC_CMD) expand2.c
#
float.o		: float.c instab.h $(COMAS)/symbols.h systems.h program.h
		$(CC_CMD) float.c
#
addr1.o		: $(COMAS)/addr1.c $(COMINC)/reloc.h $(COMINC)/syms.h \
		  $(COMINC)/storclass.h $(COMINC)/linenum.h $(COMINC)/filehdr.h \
		  gendefs.h $(COMAS)/symbols.h $(COMAS)/codeout.h systems.h
		$(CC_CMD) $(COMAS)/addr1.c
#
addr2.o		: addr2.c $(COMINC)/reloc.h $(COMINC)/storclass.h systems.h \
		  $(COMINC)/scnhdr.h $(COMINC)/syms.h gendefs.h $(COMAS)/symbols.h \
		  $(COMAS)/codeout.h instab.h
		$(CC_CMD) addr2.c
#
codeout.o	: $(COMAS)/codeout.c $(COMAS)/symbols.h $(COMAS)/codeout.h \
		  $(COMAS)/section.h gendefs.h systems.h $(COMAS)/temppack.h
		$(CC_CMD) $(COMAS)/codeout.c
#
getstab.o	: $(COMAS)/getstab.c gendefs.h $(COMAS)/symbols.h systems.h
		$(CC_CMD) $(COMAS)/getstab.c
#
pass2.o		: $(COMAS)/pass2.c $(COMAS)/codeout.h $(COMAS)/section.h gendefs.h \
		  $(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMAS)/symbols.h systems.h
		$(CC_CMD) $(COMAS)/pass2.c
#
obj.o		: $(COMAS)/obj.c $(COMINC)/filehdr.h $(COMINC)/linenum.h instab.h \
		  $(COMINC)/reloc.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		  $(COMAS)/codeout.h $(COMAS)/section.h $(COMINC)/storclass.h \
		$(COMAS)/symbols.h $(COMAS)/codeout.h gendefs.h $(M32INC)/sgs.h systems.h
		$(CC_CMD) $(COMAS)/obj.c
#
symlist.o	: $(COMAS)/symlist.c $(COMAS)/symbols.h $(COMINC)/syms.h \
		  $(COMINC)/storclass.h gendefs.h systems.h
		$(CC_CMD) $(COMAS)/symlist.c
#
symbols.o	: $(COMAS)/symbols.c $(COMAS)/symbols.h symbols2.h \
		  $(COMINC)/syms.h $(COMINC)/storclass.h systems.h
		$(CC_CMD) $(COMAS)/symbols.c
#
unused.o	: $(COMAS)/unused.c symbols2.h $(COMINC)/filehdr.h \
		  $(COMINC)/linenum.h $(COMINC)/reloc.h $(COMINC)/scnhdr.h \
		  $(COMINC)/syms.h $(COMINC)/storclass.h
		$(CC_CMD) $(COMAS)/unused.c
#
lint.out	: $(CFILES) $(HFILES) 
		$(LINT_CMD) $(DEFLIST) $(INCLIST) $(CFILES) >lint.out
		wc -l lint.out
#
install		: $(BINDIR)/$(SGS)as $(LIBDIR)/cm4defs $(LIBDIR)/cm4tvdefs
#
$(BINDIR)/$(SGS)as:	as
		cp as as.bak
		-$(STRIP_CMD) as
		sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)as as
		mv as.bak as
$(LIBDIR)/cm4defs:	cm4defs
		sh $(BASE)/sgs.install 444 $(OWN) $(GRP) $(LIBDIR)/cm4defs cm4defs
$(LIBDIR)/cm4tvdefs:	cm4tvdefs
		sh $(BASE)/sgs.install 444 $(OWN) $(GRP) $(LIBDIR)/cm4tvdefs cm4tvdefs
#
save		: $(BINDIR)/$(SGS)as $(LIBDIR)/cm4defs $(LIBDIR)/cm4tvdefs
		-rm -f $(BINDIR)/$(SGS)as.back
		cp $(BINDIR)/$(SGS)as $(BINDIR)/$(SGS)as.back
		-rm -f $(LIBDIR)/cm4defs.back $(LIBDIR)/cm4tvdefs.back
		cp $(LIBDIR)/cm4defs $(LIBDIR)/cm4defs.back
		cp $(LIBDIR)/cm4tvdefs $(LIBDIR)/cm4tvdefs.back
#
uninstall	: $(BINDIR)/$(SGS)as.back \
		  $(LIBDIR)/cm4defs.back $(LIBDIR)/cm4tvdefs.back
		-rm -f $(BINDIR)/$(SGS)as
		cp $(BINDIR)/$(SGS)as.back $(BINDIR)/$(SGS)as
		-rm -f $(LIBDIR)/cm4defs $(LIBDIR)/cm4tvdefs
		cp $(LIBDIR)/cm4defs.back $(LIBDIR)/cm4defs
		cp $(LIBDIR)/cm4tvdefs.back $(LIBDIR)/cm4tvdefs
#
shrink		:
		-rm -f $(OFILES) lint.out ind.out typ.out mnm.out sem.out make_tables
		-rm -f as
#
listall		: $(CFILES) $(HFILES)
		pr -f $(HFILES) $(CFILES) | lp

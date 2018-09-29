#	@(#)makefile	2.7	

#***************************************************************************
#                                                                          *
#		VAX 11/780 assembler makefile				   *
#									   *
#			(and VAX 11/750 assembler as well)		   *
#                                                                          *
#                                                                          *
#***************************************************************************
#                                                                          *
#		Header/Data Files					   *
#                                                                          *
#	com/symbols.h	symbol table definitions			   *
#	symbols2.h	instruction set info for the symbol table	   *
#	instab.h	instruction table definitions			   *
#	parse.h		YACC defines from parse.y			   *
#	pass0.h		DEFINES for pass0.c				   *
#	inc/paths.h	pathnames for assembler components		   *
#	inc/sgs.h	miscellaneous defines for sgs structures	   *
#	gendefs.h	header for code generation routines		   *
#	com/codeout.h	headers for code generation onto temp files	   *
#	com/expand.h	headers for span dependent instr analysis	   *
#	expand2.h	DEFINES for expand2.c sdi analysis data		   *
#	inc/syms.h	object file header for symbol table		   *
#	inc/reloc.h	object file header for relocation info		   *
#	inc/storclass.h object file header for storage class info	   *
#	inc/filehdr.h	object file header for header info		   *
#	inc/scnhdr.h	object file header for section headers		   *
#	inc/linenum.h	object file header for line number info		   *
#                                                                          *
#	ops.out		instruction table data				   *
#                                                                          *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#                                                                          *
#	com/instab.c	instruction table initialization		   *
#	parse.y		YACC based assembly language parser		   *
#	com/pass0.c	main program for pass 0 of assembler		   *
#	com/pass1.c	main program for pass 1 of assembler		   *
#	com/symbols.c	symbol table manipulation routines		   *
#	com/errors.c	error reporting procedures			   *
#	com/code.c	temp file code generation procedures		   *
#	com/expand1.c	independent span analysis procedures		   *
#	expand2.c	dependent span analysis table data		   *
#	gencode.c	code generation routines			   *
#	com/pass2.c	main program for pass 2 of assembler		   *
#	com/addr1.c	intermediate file access routines		   *
#	addr2.c		code generation action routines			   *
#	com/codeout.c	2nd pass code generation routines		   *
#	com/getstab.c	symbol table manipulation for sdi		   *
#	com/obj.c	object file creation routines			   *
#	com/symlist.c	symbol table access routines			   *
#	                                                                   *
#***************************************************************************

LPT=dpr -mx

OUTFIL = output.tmp

ROOT =
BASE = ../..
CDIR = $(BASE)/as/common
HDIR = $(BASE)/as/vax
COMI = $(BASE)/inc/common
SGSI = $(BASE)/inc/vax
ARCH = AR32WR

BINDIR = $(ROOT)/bin

OFILES = pass0.o instab.o parse.o gencode.o pass1.o symbols.o \
	 code.o expand1.o expand2.o errors.o addr2.o pass2.o \
	 addr1.o codeout.o getstab.o obj.o symlist.o rodata.o


CFILES = $(CDIR)/pass0.c $(CDIR)/instab.c parse.y gencode.c $(CDIR)/pass1.c \
	 $(CDIR)/code.c $(CDIR)/expand1.c expand2.c $(CDIR)/errors.c addr2.c \
	 $(CDIR)/pass2.c $(CDIR)/addr1.c $(CDIR)/codeout.c $(CDIR)/getstab.c \
	 $(CDIR)/obj.c $(CDIR)/symbols.c $(CDIR)/symlist.c



HFILES = pass0.h $(CDIR)/symbols.h instab.h ops.out parse.h \
         symbols2.h $(CDIR)/codeout.h $(CDIR)/expand.h \
	 $(SGSI)/sgs.h expand2.h gendefs.h systems.h \
	 $(COMI)/reloc.h $(COMI)/syms.h $(COMI)/storclass.h \
	 $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/linenum.h


CC = cc
CFLAGS =  -O -s
FLEX= -DFLEXNAMES
PPFLAGS = $(FLEX) -D$(ARCH) -I$(CDIR) -I$(HDIR) -I$(COMI) -I$(SGSI)



all: 	as


as:	$(OFILES)
	$(CC) $(CFLAGS) $(OFILES) -o as

install:	as
	cp as $(BINDIR)/as


# ASSEMBLER Components


instab.o:instab.h $(CDIR)/symbols.h ops.out parse.o systems.h $(CDIR)/instab.c
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/instab.c

parse.o:parse.y instab.h $(CDIR)/symbols.h gendefs.h
	yacc -d parse.y
	-chmod +x ./:yyfix
	./:yyfix > rodata.c
	$(CC) -DYYDEBUG=0 $(CFLAGS) -S rodata.c
	-chmod +x ./:rofix
	./:rofix rodata.s
	$(CC) -c rodata.s
	-rm -f rodata.s rodata.c
	$(CC) $(CFLAGS) $(PPFLAGS) -DYYDEBUG=0 -c y.tab.c
	cp y.tab.o parse.o
	cp y.tab.h parse.h
	-rm -f y.tab.c y.tab.h y.tab.o

parse.h:parse.o

pass0.o:$(CDIR)/pass0.c $(SGSI)/paths.h pass0.h $(SGSI)/sgs.h systems.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/pass0.c

pass1.o:$(CDIR)/pass1.c $(SGSI)/paths.h $(CDIR)/symbols.h gendefs.h \
	systems.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/pass1.c

gencode.o:gencode.c gendefs.h $(CDIR)/symbols.h instab.h parse.h \
	  $(CDIR)/expand.h expand2.h
	  $(CC) $(CFLAGS) $(PPFLAGS) -c gencode.c

errors.o: $(CDIR)/errors.c gendefs.h systems.h
	  $(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/errors.c

code.o:	$(CDIR)/codeout.h gendefs.h $(CDIR)/code.c $(CDIR)/symbols.h \
	systems.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/code.c

expand1.o: $(CDIR)/expand1.c $(CDIR)/expand.h $(CDIR)/symbols.h gendefs.h \
	   systems.h
	   $(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/expand1.c

expand2.o: $(CDIR)/expand.h expand2.c expand2.h $(CDIR)/symbols.h
	   $(CC) $(CFLAGS) $(PPFLAGS) -c expand2.c

pass2.o: $(CDIR)/pass2.c gendefs.h $(CDIR)/symbols.h systems.h
	 $(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/pass2.c

addr1.o: $(CDIR)/addr1.c $(COMI)/reloc.h $(COMI)/syms.h \
	 $(COMI)/storclass.h $(COMI)/linenum.h  $(COMI)/filehdr.h \
	 gendefs.h $(CDIR)/symbols.h $(CDIR)/codeout.h instab.h
	 $(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/addr1.c

addr2.o: addr2.c gendefs.h instab.h $(CDIR)/symbols.h $(CDIR)/codeout.h \
	 $(COMI)/reloc.h systems.h
	 $(CC) $(CFLAGS) $(PPFLAGS) -c addr2.c

codeout.o: $(CDIR)/codeout.c $(CDIR)/symbols.h $(CDIR)/codeout.h gendefs.h \
	   systems.h
	   $(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/codeout.c

getstab.o: $(CDIR)/getstab.c gendefs.h $(CDIR)/symbols.h
	   $(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/getstab.c

obj.o:	$(CDIR)/obj.c $(COMI)/filehdr.h $(COMI)/linenum.h \
	$(COMI)/reloc.h $(COMI)/scnhdr.h $(COMI)/syms.h $(COMI)/storclass.h \
	$(CDIR)/symbols.h $(CDIR)/codeout.h gendefs.h $(SGSI)/sgs.h \
	instab.h systems.h
	$(CC) $(CFLAGS) $(PPFLAGS) -DMAGIC=VAXROMAGIC -c $(CDIR)/obj.c

symbols.o: $(CDIR)/symbols.c $(CDIR)/symbols.h symbols2.h systems.h
	    $(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/symbols.c

symlist.o: $(CDIR)/symlist.c $(CDIR)/symbols.h $(COMI)/syms.h \
	   $(COMI)/storclass.h gendefs.h systems.h
	   $(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/symlist.c



listing:
	pr -n makefile $(HFILES) $(CFILES)  | $(LPT)
	@echo "files sent to lineprinter!"

clean:
	-rm -f $(OFILES) 

flow:
	cflow $(PPFLAGS) $(CFILES) >  $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

clobber:	clean
	-rm -f as

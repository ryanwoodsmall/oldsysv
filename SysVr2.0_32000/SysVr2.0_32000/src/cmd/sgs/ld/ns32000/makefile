#	@(#)makefile	2.11 3/25/83

#******************************************************************************
#									      *
#		NSC 32000 linkage editor makefile			      *
#									      *
#******************************************************************************
#									      *
#		Header/Data Files					      *
#									      *
#		com/attributes.h					      *
#		com/list.h						      *
#		com/structs.h						      *
#		com/extrns.h						      *
#		com/instr.h						      *
#		system.h						      *
#		params.h						      *
#		inc/filehdr.h						      *
#		inc/scnhdr.h						      *
#		define2.h						      *
#		inc/syms.h						      *
#		inc/storclass.h						      *
#		bool.h							      *
#		inc/sgsmacros.h						      *
#		inc/sgs.h						      *
#		inc/paths.h						      *
#		inc/ldfcn.h						      *
#		inc/reloc.h						      *
#		inc/linenum.h						      *
#		patch.h							      *
#		inc/aouthdr.h						      *
#		com/slotvec.h						      *
#									      *
#******************************************************************************
#									      *
#		Source Code Files					      *
#									      *
#		com/ld.yac	yacc input specifications		      *
#		com/ld.lex	lex input specifications		      *
#		com/expr0.c	build parse trees of expressions	      *
#		com/expr1.c	evaluate parse trees of expressions	      *
#		com/ld00.c	process ld flags			      *
#		com/ld01.c	main routine of ld pass 2		      *
#		com/ld1.c	load .o files, do library search	      *
#		com/ld2.c	misc. routines for special features	      *
#		com/alloc.c	allocation of sections			      *
#		com/output.c	output a.out file and do relocation	      *
#		com/maps.c	produce object file mappings		      *
#		com/util.c	ld utility routines			      *
#		com/lists.c	handle linked list structures		      *
#		com/dump.c	dump information for debugging		      *
#		com/syms.c	symbol table processing routines	      *
#		com/globs.c	process global variables		      *
#		com/instr.c	output symbol table run time statistics	      *
#		com/slotvec.c	slot vector processing routines		      *
#		com/version.c	ld version control			      *
#		special.c	special NSC system logic		      *
#									      *
#******************************************************************************



OUTFIL = output.tmp

ROOT =
SGSBASE = /usr/src/cmd/sgs
CDIR = ../common
HDIR = .
SGSI = $(SGSBASE)/inc/ns32000
COMI = $(SGSBASE)/inc/common

BINDIR = $(ROOT)/bin

SGS =
ARCH = AR32WR
# Specify the archive format of the host machine
#	PORTAR : 6.0 portable archive format
#	PORT5AR : 5.0 archive format
#	OLDAR : pre-5.0 archive format
#
ARFORMAT = PORTAR
FLEX = -DFLEXNAMES
LIBLD = $(SGSBASE)/libld/common/libld.a
LDFLAGS = -s
B10 =

CC = cc
CFLAGS = -O

YACC = yacc
YFLAGS = -d

LEX = lex
LEXLIB = /usr/lib/libl.a
LEXFLAGS =

LINT = lint
LINTFLAGS  = -abhxp
LINTOUT = lint.out

PRINT = pr
PRFLAGS = -n
LP = lp

RM = rm
CP = cp

ENV =
FRC =

DEFLIST = -D$(ARFORMAT) $(FLEX) -D$(ARCH)
INCLIST = -I$(HDIR) -I$(CDIR) -I$(SGSI) -I$(COMI) -I/usr/include

LIBES = $(LEXLIB) -ly $(LIBLD)

STRIP	= strip
SPFLAGS =

OFILES = rodata.o expr0.o globs.o ld00.o lex.yy.o lists.o\
	 util.o y.tab.o special.o\
	 alloc.o dump.o expr1.o ld01.o ld1.o ld2.o\
	 version.o maps.o output.o slotvec.o syms.o instr.o


CFILES = $(CDIR)/ld.yac $(CDIR)/ld.lex $(CDIR)/expr0.c $(CDIR)/expr1.c\
	 $(CDIR)/ld00.c $(CDIR)/ld01.c $(CDIR)/ld1.c $(CDIR)/ld2.c\
	 $(CDIR)/alloc.c $(CDIR)/output.c $(CDIR)/util.c $(CDIR)/lists.c\
	 $(CDIR)/dump.c $(CDIR)/syms.c $(CDIR)/globs.c $(CDIR)/instr.c\
	 $(CDIR)/slotvec.c $(CDIR)/version.c $(CDIR)/maps.c \
	 special.c


HFILES = $(CDIR)/attributes.h $(CDIR)/list.h $(CDIR)/structs.h\
	 $(CDIR)/ldmacros.h $(CDIR)/extrns.h $(CDIR)/instr.h system.h params.h\
	 $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/sdpsrc/hd/define2.h \
	 $(COMI)/syms.h\
	 $(COMI)/storclass.h bool.h $(COMI)/sgsmacros.h\
	 $(SGSI)/sgs.h $(SGSI)/paths.h $(COMI)/ldfcn.h\
	 $(COMI)/reloc.h $(COMI)/linenum.h $(COMI)/patch.h\
	 $(COMI)/aouthdr.h $(CDIR)/slotvec.h


all:	ld



ld:	$(OFILES) $(LIBLD)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OFILES) $(LIBES) -o ld

install:	ld
	-$(RM) -f $(BINDIR)/$(SGS)ld
	$(CP) ld $(BINDIR)/$(SGS)ld
	$(STRIP) $(SPFLAGS) $(BINDIR)/$(SGS)ld

#	linkage editor components


y.tab.c y.tab.h rodata.o:	$(CDIR)/ld.yac
	@echo "Expect to see message 'conflicts: 6 shift/reduce'"
	$(YACC) $(YFLAGS) $(CDIR)/ld.yac
	-chmod +x ./:yyfix
	./:yyfix  > rodata.c
	$(CC) -DYYDEBUG=0 $(CFLAGS) -S rodata.c
	if vax || ns32000; then \
		sh ./:rofix rodata.s; \
	fi
	$(CC) -c rodata.s
	-$(RM) -f rodata.s rodata.c

y.tab.o:	$(CDIR)/attributes.h $(CDIR)/list.h $(CDIR)/structs.h $(FRC)\
		$(CDIR)/ldmacros.h $(CDIR)/extrns.h system.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h bool.h $(COMI)/sgsmacros.h\
		$(SGSI)/sgs.h y.tab.c
	$(CC) $(B10) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) -DYYDEBUG=0 y.tab.c

lex.yy.c:	$(CDIR)/ld.lex
	$(LEX) $(CDIR)/ld.lex

lex.yy.o:	$(CDIR)/structs.h $(CDIR)/extrns.h system.h $(FRC)\
		$(CDIR)/instr.h $(COMI)/aouthdr.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h bool.h\
		$(SGSI)/paths.h $(SGSI)/sgs.h \
		y.tab.h lex.yy.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) lex.yy.c

expr0.o:	$(CDIR)/ldmacros.h $(CDIR)/extrns.h $(FRC)\
		$(CDIR)/structs.h system.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/sgsmacros.h y.tab.h\
		$(CDIR)/expr0.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/expr0.c

expr1.o:	$(CDIR)/ldmacros.h $(CDIR)/extrns.h $(FRC)\
		$(CDIR)/structs.h system.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/sgsmacros.h y.tab.h\
		$(CDIR)/expr1.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/expr1.c

ld00.o:		$(CDIR)/extrns.h $(CDIR)/list.h $(CDIR)/structs.h $(FRC)\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/sgsmacros.h\
		$(SGSI)/paths.h $(SGSI)/sgs.h\
 		system.h y.tab.h $(CDIR)/ld00.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/ld00.c

ld01.o:		$(CDIR)/ldmacros.h $(CDIR)/extrns.h $(CDIR)/structs.h $(FRC)\
		$(CDIR)/list.h $(CDIR)/instr.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/sgsmacros.h system.h\
		$(CDIR)/ld01.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/ld01.c

ld1.o:		$(CDIR)/extrns.h $(CDIR)/ldmacros.h $(CDIR)/list.h $(FRC)\
		$(CDIR)/structs.h params.h system.h $(SGSI)/sgs.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/ldfcn.h $(COMI)/reloc.h\
		$(COMI)/sgsmacros.h $(SGSI)/sgs.h $(CDIR)/ld1.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/ld1.c

ld2.o:		$(CDIR)/extrns.h $(CDIR)/list.h $(FRC)\
		$(CDIR)/structs.h system.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/sgsmacros.h $(COMI)/storclass.h\
		$(COMI)/reloc.h $(COMI)/linenum.h $(COMI)/patch.h\
		params.h y.tab.h $(CDIR)/ld2.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/ld2.c

alloc.o:	$(CDIR)/extrns.h $(CDIR)/list.h $(CDIR)/structs.h $(FRC)\
		$(CDIR)/ldmacros.h params.h system.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/sgsmacros.h\
		$(CDIR)/alloc.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/alloc.c

output.o:	$(CDIR)/ldmacros.h $(CDIR)/structs.h $(FRC)\
		$(CDIR)/extrns.h system.h\
		$(CDIR)/slotvec.h $(CDIR)/instr.h $(COMI)/aouthdr.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/sgsmacros.h $(COMI)/storclass.h params.h\
		$(COMI)/reloc.h $(COMI)/linenum.h $(COMI)/ldfcn.h\
		$(COMI)/patch.h $(SGSI)/sgs.h $(CDIR)/output.c
	$(CC) $(B10) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/output.c

maps.o:		$(CDIR)/extrns.h $(CDIR)/structs.h $(CDIR)/attributes.h $(FRC)\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/sgsmacros.h\
		system.h $(CDIR)/maps.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/maps.c

util.o:		$(CDIR)/structs.h $(CDIR)/extrns.h system.h $(FRC)\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/sgsmacros.h\
		$(SGSI)/sgs.h $(CDIR)/util.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/util.c

lists.o: 	$(CDIR)/list.h $(CDIR)/structs.h system.h $(FRC)\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/sgsmacros.h $(COMI)/storclass.h\
		$(CDIR)/lists.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/lists.c

dump.o:		$(CDIR)/structs.h $(CDIR)/extrns.h system.h $(FRC)\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(CDIR)/dump.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/dump.c

syms.o:		$(CDIR)/ldmacros.h $(FRC)\
		$(CDIR)/structs.h $(CDIR)/extrns.h $(CDIR)/instr.h\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h $(COMI)/sgsmacros.h system.h\
		$(SGSI)/paths.h $(CDIR)/syms.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/syms.c

globs.o:	$(CDIR)/structs.h params.h system.h $(FRC)\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/syms.h\
		$(COMI)/storclass.h\
		$(SGSI)/paths.h $(CDIR)/globs.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/globs.c

instr.o:	$(CDIR)/instr.c $(FRC)
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/instr.c

slotvec.o:	$(CDIR)/slotvec.h $(CDIR)/structs.h $(CDIR)/extrns.h $(FRC)\
		$(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/sdpsrc/hd/define2.h\
		$(COMI)/syms.h $(SGSI)/paths.h\
       		$(SGSI)/paths.h system.h $(COMI)/storclass.h\
		$(CDIR)/slotvec.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/slotvec.c

version.o:	$(CDIR)/version.c $(FRC)
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) $(CDIR)/version.c


special.o:	$(COMI)/aouthdr.h $(CDIR)/structs.h params.h $(FRC)\
		$(COMI)/ldfcn.h\
		$(CDIR)/slotvec.h $(COMI)/reloc.h $(CDIR)/extrns.h\
		$(CDIR)/list.h system.h $(SGSI)/sgs.h\
		$(COMI)/sgsmacros.h $(COMI)/syms.h special.c
	$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST) $(ENV) special.c



# Force recompilation
FRC:


listing:
	$(PR) $(PRFLAGS) makefile $(HFILES) $(CFILES) | $(LP)
	@echo "files sent to printer!"


clean:
	-$(RM) -f $(OFILES) y.tab.[ch] lex.yy.c

flow:
	cflow $(DEFLIST) $(INCLIST) $(ENV) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(DEFLIST) $(INCLIST) $(ENV) $(CFILES) > $(OUTFIL)

clobber: clean
	-$(RM) -f ld

lint:
	$(CFILES) $(HFILES)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(ENV) $(CFILES) > $(LINTOUT)

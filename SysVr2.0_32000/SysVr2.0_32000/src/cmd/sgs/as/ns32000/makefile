#	@(#)makefile	2.7	

#***************************************************************************
#                                                                          *
#		NSC System-32 assembler makefile			   *
#									   *
#                                                                          *
#                                                                          *
#***************************************************************************
#                                                                          *
#		Header/Data Files					   *
#                                                                          *
#                                                                          *
#                                                                          *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#                                                                          *
#	                                                                   *
#***************************************************************************

LPT=dpr -mx

OUTFIL = output.tmp

ROOT =
ARCH = AR32WR

BINDIR = $(ROOT)/bin

OFILES = as.parse.o as.sym.o as.gen.o as.pass.o as.hdr.o as.cbuf.o as.cpy.o \
	as.opt.o as.main.o


CFILES = as.sym.c as.gen.c as.pass.c as.hdr.c as.cbuf.c as.cpy.c as.main.c \
	as.opt.c as.parse.y



HFILES = as.addr.h as.cbuf.h as.ops.h as.rel.h as.sym.h


CFLAGS =  -O
FLEX= -DFLEXNAMES
PPFLAGS = $(FLEX) -D$(ARCH)



all: 	as


as:	$(OFILES)
	$(CC) $(CFLAGS) $(OFILES) -o as /usr/lib/libl.a

install:	as
	cp as $(BINDIR)/as


# ASSEMBLER Components


lex.yy.c: as.scan.l
	lex -n as.scan.l

as.parse.o: lex.yy.c as.parse.y
	yacc -d as.parse.y
	ed y.tab.c <:yyfix
	$(CC) $(CFLAGS) $(PPFLAGS) -DYYDEBUG=0 -c y.tab.c
	-rm y.tab.c
	mv y.tab.o as.parse.o

listing:
	pr -n makefile $(HFILES) $(CFILES)  | $(LPT)
	@echo "files sent to lineprinter!"

clean:
	-rm -f $(OFILES)  y.tab.c y.tab.o

flow:
	cflow $(PPFLAGS) $(CFILES) >  $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

clobber:	clean
	-rm -f as lex.yy.c

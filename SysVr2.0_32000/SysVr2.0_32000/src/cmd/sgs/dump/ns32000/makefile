#	@(#) makefile: 2.6 3/23/83

#*****************************************************************************
#									     *
#		UNIX Mach Lang File examination (dump) makefile 	     *
#									     *
#*****************************************************************************
#									     *
#		Header/Data Files				   	     *
#									     *
#		com/process.h						     *
#		com/filedefs.h						     *
#		com/flagdefs.h						     *
#									     *
#*****************************************************************************
#									     *
#		Source Code Files					     *
#									     *
#		com/main.c						     *
#		com/setflags.c						     *
#		com/openobject.c					     *
#		com/process.c						     *
#		com/fcns.c						     *
#									     *
#*****************************************************************************
#									     *
#		Library Files						     *
#									     *
#		lib/libld.a						     *
#									     *
#*****************************************************************************


ROOT =
BASE = ../../
LIBLD =  $(BASE)/libld/common/libld.a
SGSI = $(BASE)/inc/ns32000
COMI = $(BASE)/inc/common
CDIR = $(BASE)/dump/common
ARCH = AR32WR
FLEX = -DFLEXNAMES
SGS =
BINDIR = $(ROOT)/bin
B01 =

LPT = dpr -mx

OUTFIL = output.tmp

CFLAGS = -O -s
PPFLAGS = -DPORTAR -D$(ARCH) $(FLEX) -I$(SGSI) -I$(COMI)


CFILES = $(CDIR)/main.c $(CDIR)/setflags.c $(CDIR)/openobject.c \
	 $(CDIR)/process.c $(CDIR)/fcns.c

HFILES = $(CDIR)/process.h $(CDIR)/filedefs.h $(CDIR)/flagdefs.h \
	 $(COMI)/filehdr.h $(COMI)/ldfcn.h $(COMI)/scnhdr.h \
	 $(COMI)/reloc.h $(COMI)/linenum.h $(COMI)/syms.h  \
	 $(COMI)/storclass.h $(SGSI)/sgs.h


OFILES = main.o setflags.o openobject.o process.o fcns.o

all:	dump

dump:  $(OFILES) $(LIBLD)
	$(CC) $(CFLAGS) -o dump $(OFILES) $(LIBLD)

main.o:  $(CDIR)/main.c $(COMI)/filehdr.h $(COMI)/ldfcn.h $(CDIR)/filedefs.h \
	 $(SGSI)/sgs.h
	$(CC) $(CFLAGS) -c $(PPFLAGS) $(CDIR)/main.c

setflags.o:  $(CDIR)/setflags.c $(CDIR)/flagdefs.h $(SGSI)/sgs.h
	$(CC) $(CFLAGS) -c $(PPFLAGS) $(CDIR)/setflags.c

openobject.o:  $(CDIR)/openobject.c $(COMI)/filehdr.h $(COMI)/ldfcn.h \
	       $(SGSI)/sgs.h $(CDIR)/filedefs.h
	$(CC) $(CFLAGS) -c $(PPFLAGS) $(CDIR)/openobject.c

process.o:  $(CDIR)/process.c $(COMI)/filehdr.h $(COMI)/ldfcn.h $(SGSI)/sgs.h\
	    $(CDIR)/filedefs.h $(CDIR)/process.h $(CDIR)/flagdefs.h
	$(CC) $(CFLAGS) -c $(PPFLAGS) $(CDIR)/process.c

fcns.o:  $(CDIR)/fcns.c $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/reloc.h \
	 $(COMI)/linenum.h $(COMI)/syms.h $(COMI)/storclass.h $(COMI)/ldfcn.h \
	 $(SGSI)/sgs.h
	$(CC) $(B01) $(CFLAGS) -c $(PPFLAGS) $(CDIR)/fcns.c

install:	dump
	cp dump $(BINDIR)/$(SGS)dump

clean:	
	-rm -f $(OFILES)

listing:
	pr -n makefile $(HFILES) $(CFILES) | $(LPT)
	@echo "files sent to lineprinter!"

flow:
	cflow $(PPFLAGS) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

clobber: clean
	-rm -f dump

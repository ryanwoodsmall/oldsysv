#	@(#) makefile: 2.6 3/5/83

#******************************************************************************
#									      *
#		UNIX Symbol Table Examination (nm) Makefile		      *
#									      *
#******************************************************************************
#									      *
#		Header/Data Files					      *
#									      *
#		com/decotype.h						      *
#		com/defs.h						      *
#		com/prosym.h						      *
#									      *
#******************************************************************************
#									      *
#		Source Code Files					      *
#									      *
#		com/decotype.c						      *
#		com/fcns.c						      *
#		com/main.c						      *
#		com/process.c						      *
#		com/prosym.c						      *
#		com/setflags.c						      *
#									      *
#******************************************************************************
#									      *
#		Library Files						      *
#									      *
#		/lib/libld.a						      *
#									      *
#******************************************************************************


ROOT =
BASE = ../..
BINDIR = $(ROOT)/bin
ARCH = AR32WR
SGS =
LIBLD =  $(BASE)/libld/common/libld.a
COMI = $(BASE)/inc/common
SGSI = $(BASE)/inc/ns32000
FLEX = -DFLEXNAMES

CDIR = $(BASE)/nm/common

LPT = dpr -mx

OUTFIL = output.tmp

CFLAGS = -O -s
PPFLAGS = $(FLEX) -DPORTAR -DUNIX -D$(ARCH) -I$(SGSI) -I$(COMI)



CFILES = $(CDIR)/decotype.c $(CDIR)/fcns.c $(CDIR)/main.c \
	 $(CDIR)/process.c $(CDIR)/prosym.c $(CDIR)/setflags.c

HFILES = $(CDIR)/decotype.h $(CDIR)/defs.h $(CDIR)/prosym.h \
	 $(COMI)/syms.h $(COMI)/storclass.h \
	 $(COMI)/filehdr.h $(COMI)/ldfcn.h $(SGSI)/sgs.h $(SGSI)/paths.h

OFILES = main.o setflags.o process.o prosym.o decotype.o fcns.o



all:	nm


nm:	$(OFILES) $(HFILES) $(LIBLD)
	$(CC) $(CFLAGS) -o nm $(OFILES) $(LIBLD)

install:	nm
	cp nm $(BINDIR)/$(SGS)nm

main.o:  $(CDIR)/main.c $(COMI)/filehdr.h $(COMI)/ldfcn.h $(SGSI)/sgs.h \
	 $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/main.c

setflags.o:  $(CDIR)/setflags.c $(SGSI)/sgs.h $(SGSI)/paths.h $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/setflags.c

process.o:  $(CDIR)/process.c $(COMI)/filehdr.h $(COMI)/ldfcn.h \
	    $(COMI)/scnhdr.h $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/process.c

prosym.o:  $(CDIR)/prosym.c $(COMI)/filehdr.h $(COMI)/ldfcn.h $(COMI)/syms.h \
	   $(COMI)/storclass.h $(CDIR)/defs.h $(CDIR)/prosym.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/prosym.c

decotype.o:  $(CDIR)/decotype.c $(COMI)/filehdr.h $(COMI)/ldfcn.h \
	     $(COMI)/syms.h $(COMI)/storclass.h $(CDIR)/decotype.h \
	     $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/decotype.c

fcns.o:  $(CDIR)/fcns.c $(COMI)/filehdr.h $(COMI)/ldfcn.h $(SGSI)/sgs.h \
	 $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/fcns.c


clean:	
	-rm -f $(OFILES)

clobber:	clean
	-rm -f nm
listing:
	pr -n makefile $(HFILES) $(CFILES) | $(LPT)
	@echo "files sent to lineprinter!"

flow:
	cflow $(PPFLAGS) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)


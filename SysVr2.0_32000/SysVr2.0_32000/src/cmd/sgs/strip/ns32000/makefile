#	@(#) makefile: 2.7 3/23/83

#******************************************************************************
#									      *
#		UNIX Mach Lang File Stripper (strip) makefile		      *
#									      *
#******************************************************************************
#									      *
#		Header/Data Files					      *
#									      *
#		com/defs.h						      *
#									      *
#******************************************************************************
#									      *
#		Source Code Files					      *
#									      *
#		com/main.c						      *
#		com/setflags.c						      *
#		com/process.c						      *
#		com/checkflags.c					      *
#		com/buildtmp.c						      *
#		com/buildfcns.c						      *
#		com/copytmp.c						      *
#		com/fcns.c						      *
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
LIBLD = $(BASE)/libld/common/libld.a
SGS = 
ARCH = AR32WR
BINDIR = $(ROOT)/bin
COMI = $(BASE)/inc/common
SGSI = $(BASE)/inc/ns32000
FLEX = -DFLEXNAMES

CDIR = $(BASE)/strip/common

LPT = dpr -mx

OUTFIL = output.tmp

CFLAGS = -O -s
PPFLAGS = -DPORTAR -DUNIX -D$(ARCH) $(FLEX) -I$(CDIR) -I$(SGSI) -I$(COMI)


CFILES = $(CDIR)/main.c $(CDIR)/setflags.c $(CDIR)/process.c \
	 $(CDIR)/checkflags.c $(CDIR)/buildtmp.c $(CDIR)/buildfcns.c \
	 $(CDIR)/copytmp.c $(CDIR)/fcns.c

HFILES = $(CDIR)/defs.h $(COMI)/filehdr.h $(COMI)/ldfcn.h $(COMI)/scnhdr.h \
         $(COMI)/reloc.h $(COMI)/syms.h $(COMI)/storclass.h \
	 $(SGSI)/paths.h $(SGSI)/sgs.h

OFILES = main.o setflags.o process.o checkflags.o buildtmp.o buildfcns.o \
	 copytmp.o fcns.o


all:	strip

strip: $(OFILES) $(HFILES) $(LIBLD)
	$(CC) $(CFLAGS) $(PPFLAGS) -o strip $(OFILES) $(LIBLD)



main.o:  $(CDIR)/main.c $(COMI)/filehdr.h $(COMI)/ldfcn.h $(SGSI)/paths.h \
	 $(SGSI)/sgs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/main.c

setflags.o:  $(CDIR)/setflags.c $(SGSI)/sgs.h $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/setflags.c

process.o:  $(CDIR)/process.c $(COMI)/filehdr.h $(COMI)/ldfcn.h \
	    $(CDIR)/defs.h $(SGSI)/paths.h $(SGSI)/sgs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/process.c

checkflags.o: $(CDIR)/checkflags.c $(COMI)/filehdr.h $(COMI)/ldfcn.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/checkflags.c

buildtmp.o:  $(CDIR)/buildtmp.c $(COMI)/filehdr.h $(COMI)/ldfcn.h \
	     $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/buildtmp.c

buildfcns.o:  $(CDIR)/buildfcns.c $(COMI)/filehdr.h $(COMI)/scnhdr.h \
	      $(COMI)/reloc.h $(COMI)/syms.h $(COMI)/ldfcn.h \
	      $(COMI)/storclass.h $(CDIR)/defs.h $(SGSI)/sgs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/buildfcns.c

copytmp.o:  $(CDIR)/copytmp.c $(COMI)/filehdr.h $(COMI)/ldfcn.h \
	    $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/copytmp.c

fcns.o:  $(CDIR)/fcns.c $(COMI)/filehdr.h $(COMI)/ldfcn.h $(SGSI)/sgs.h \
	 $(CDIR)/defs.h
	$(CC) $(CFLAGS) $(PPFLAGS) -c $(CDIR)/fcns.c


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
	-rm -f strip

install:	strip
	cp strip $(BINDIR)/$(SGS)strip

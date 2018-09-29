#	@(#) makefile: 2.5 11/28/82

#*****************************************************************************
#									     *
#		UNIX Mach Lang File size (size) makefile		     *
#									     *
#*****************************************************************************
#									     *
#		Header/Data Files					     *
#									     *
#		com/process.h 						     *
#		com/defs.h						     *
#									     *
#*****************************************************************************
#									     *
#		Source Code Files					     *
#									     *
#		com/main.c						     *
#		com/process.c						     *
#		com/fcns.c						     *
#									     *
#*****************************************************************************
#									     *
#		Library Files						     *
#									     *
#		/lib/libld.a						     *
#									     *
#*****************************************************************************


ROOT =
BASE = ../../
LIBLD = $(BASE)/libld/common/libld.a
SGS =
ARCH = AR32WR
BINDIR = $(ROOT)/bin
SGSI = $(BASE)/inc/ns32000
COMI = $(BASE)/inc/common

CDIR = $(BASE)/size/common

LPT = dpr -mx

OUTFIL = output.tmp

CFLAGS = -O -s
PPFLAGS = -DPORTAR -DUNIX -D$(ARCH) -I$(SGSI) -I$(COMI)


CFILES = $(CDIR)/main.c $(CDIR)/process.c $(CDIR)/fcns.c

HFILES = $(CDIR)/process.h $(CDIR)/defs.h $(COMI)/filehdr.h  \
	 $(COMI)/ldfcn.h $(COMI)/scnhdr.h  $(SGSI)/sgs.h

OFILES = main.o process.o fcns.o


all:	size


size:  $(OFILES)  $(HFILES) $(LIBLD)
	$(CC) $(CFLAGS) -o size $(OFILES) $(LIBLD)


main.o:   $(COMI)/filehdr.h $(COMI)/ldfcn.h $(SGSI)/sgs.h  \
	  $(CDIR)/defs.h $(CDIR)/main.c
	$(CC) $(CFLAGS) -c $(PPFLAGS) $(CDIR)/main.c

process.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h \
	    $(CDIR)/process.h $(CDIR)/defs.h $(CDIR)/process.c
	$(CC) $(CFLAGS) -c $(PPFLAGS) $(CDIR)/process.c

fcns.o:  $(COMI)/filehdr.h $(COMI)/scnhdr.h $(COMI)/ldfcn.h \
	 $(SGSI)/sgs.h $(CDIR)/defs.h $(CDIR)/fcns.c
	$(CC) $(CFLAGS) -c $(PPFLAGS) $(CDIR)/fcns.c


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
	-rm -f size

install:	size
	cp size $(BINDIR)/$(SGS)size

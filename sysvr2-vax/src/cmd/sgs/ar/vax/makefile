#	@(#) makefile: 2.5 3/5/83

#***************************************************************************
#                                                                          *
#			UNIX Archive (ar) makefile			   *
#									   *
#***************************************************************************
#                                                                          *
#		Header/Data Files					   *
#                                                                          *
#	inc/ar.h		archive file format header		   *
#	inc/sgs.h		sgs primitives				   *
#                                                                          *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#	com/ar.c		archive source code			   *
#									   *
#***************************************************************************

LPT = dpr -mx

OUTFIL = output.tmp

ROOT =
BASE = ../..
COMI = $(BASE)/inc/common
CDIR = $(BASE)/ar/common
USRINC	= $(ROOT)/usr/include

LIBLD = $(BASE)/libld/common/libld.a
BINDIR = $(ROOT)/bin
ARCH = AR32WR
FLEX = -DFLEXNAMES
SGS = 

STRIP	= strip

OFILES = ar.o

CFILES = $(CDIR)/ar.c

HFILES = $(USRINC)/ar.h $(COMI)/filehdr.h $(COMI)/syms.h

CFLAGS =  -O 

PPFLAGS = $(FLEX) -D$(ARCH) -I$(COMI) -I$(USRINC)


all:	ar

ar:	$(CFILES) $(HFILES) $(LIBLD)
	$(CC) $(CFLAGS) $(PPFLAGS) $(CFILES) -o ar $(LIBLD)

install: ar
	cp ar $(BINDIR)/$(SGS)ar
	$(STRIP) $(BINDIR)/$(SGS)ar

listing:
	pr -n makefile $(HFILES) $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f $(OFILES)

flow:
	cflow $(PPFLAGS) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

clobber: clean
	-rm -f ar

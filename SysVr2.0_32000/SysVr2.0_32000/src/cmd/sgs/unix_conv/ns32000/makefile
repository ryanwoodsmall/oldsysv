#	@(#)makefile	2.5 3/5/83

#***************************************************************************
#                                                                          *
#		UNIX 5.0 Transition Aid (convert) makefile	 	   *
#									   *
#***************************************************************************
#                                                                          *
#		Header/Data Files					   *
#                                                                          *
#	inc/ar.h		archive file format header		   *
#	inc/a.out.h		machine language file format header        *
#	inc/sgs.h		sgs primitives				   *
#	com/old.a.out.h		pre 5.0 headers for a.out.h		   *
#	com/old.ar.h		pre 5.0 headers for ar.h		   *
#                                                                          *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#	com/convert.c		transition aid source code		   *
#									   *
#***************************************************************************

LPT = dpr -mx

OUTFIL = output.tmp

ROOT	=
BINDIR	= $(ROOT)/bin

CDIR = ../common
USRINC	= $(ROOT)/usr/include

OFILES = convert.o

CFILES = $(CDIR)/convert.c

HFILES = $(USRINC)/ar.h $(USRINC)/a.out.h \
	 $(CDIR)/old.a.out.h $(CDIR)/old.ar.h

LIBLD = -lld

CFLAGS = -O 
STRIP	= strip

FLEX	= -DFLEXNAMES

PPFLAGS = -I$(USRINC) -I$(SGSI) -I$(CDIR) $(FLEX)


all:	convert


convert:$(CFILES) $(HFILES)
	$(CC) $(PPFLAGS) $(CFLAGS) $(CFILES) $(LIBLD) -o convert

install:	convert
	cp convert $(BINDIR)/convert
	$(STRIP) $(BINDIR)/convert


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
	-rm -f convert

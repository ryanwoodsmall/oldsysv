#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)unix_conv:m32/makefile	1.10"

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

OWN	= bin
GRP	= bin
ROOT	=
BINDIR	= $(ROOT)/bin

COMI = ../../inc/common
CDIR = ../common
INCM32 = ../../inc/m32

LDLIBS=

OFILES = convert.o

CFILES = $(CDIR)/convert.c

HFILES = $(COMI)/ar.h $(COMI)/aouthdr.h $(COMI)/filehdr.h \
	 $(COMI)/scnhdr.h $(COMI)/linenum.h $(COMI)/reloc.h \
	 $(COMI)/syms.h $(COMI)/storclass.h \
	 $(CDIR)/old.a.out.h $(CDIR)/old.ar.h $(CDIR)/5.0.ar.h \
	 $(INCM32)/paths.h

BASE	= ../..
LIBLD	= $(BASE)/libld/m32/libld.a
STRIP	= strip

FLEX	= -DFLEXNAMES
DEFLIST	= -DM32 $(FLEX) -D$(ARFORMAT)
INCLIST	= -I$(CDIR) -I$(COMI) -I$(INCM32)

CFLAGS = -O

ARFORMAT=PORTAR


all:	convert


convert:$(CFILES) $(HFILES) $(LIBLD)
	$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) $(CFILES) $(LIBLD) $(LDLIBS) -o convert


install:	convert
	cp convert convert.bak
	$(STRIP) convert
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/convert convert
	mv convert.bak convert

listing:
	pr -n makefile $(HFILES) $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f $(OFILES)

flow:
	cflow $(PPFLAGS) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

shrink:  clobber

clobber: clean
	-rm -f convert

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ar:m32/makefile	1.11"

#***************************************************************************
#                                                                          *
#			UNIX Archive (ar) makefile			   *
#									   *
#***************************************************************************

ROOT	=
BINDIR	= $(ROOT)/bin
BASE	= ../..
COMI	= $(BASE)/inc/common
INCM32	= $(BASE)/inc/m32
CDIR	= $(BASE)/ar/common
FLEX	= -DFLEXNAMES
ARFORMAT= PORTAR

OWN	= bin
GRP	= bin
STRIP	= strip
LP	= lp

USRINC= $(ROOT)/usr/$(VERSION)include

LIBLD	= $(BASE)/libld/m32/libld.a

CFILES = $(CDIR)/ar.c

HFILES = $(COMI)/ar.h $(COMI)/filehdr.h $(COMI)/syms.h $(INCM32)/paths.h

CFLAGS =  -O 
LDLIBS =
DEFLIST	= $(FLEX) -D$(ARFORMAT)
INCLIST	= -I$(COMI) -I$(INCM32) -I$(USRINC)


all:	ar


ar:	$(CFILES) $(HFILES) $(LIBLD)
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(CFILES) -o ar $(LIBLD) $(LDLIBS)

install:	ar
	cp ar ar.bak
	$(STRIP) ar
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/ar ar
	mv ar.bak ar

listing:
	pr -n makefile $(HFILES) $(CFILES) | $(LP)
	@echo "files sent to lineprinter"

clean:

flow:
	cflow $(PPFLAGS) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

shrink:		clobber

clobber:	clean
	-rm -f ar

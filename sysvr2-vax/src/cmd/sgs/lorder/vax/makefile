#	@(#) makefile: 2.5 12/11/82

#***************************************************************************
#                                                                          *
#			UNIX Lorder (lorder) makefile			   *
#									   *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#	com/lorder		lorder function shell script		   *
#									   *
#***************************************************************************

LPT = dpr -mx

ROOT =
BASE = ../..
CDIR = $(BASE)/lorder/common

BINDIR = $(ROOT)/bin
SGS =
ARCH = AR32WR

SFILES = $(CDIR)/lorder


all:	lorder


lorder:	$(SFILES)
	sed 's/SGS/$(SGS)/' < $(CDIR)/lorder > lorder
	chmod +x lorder


listing:
	pr -n makefile $(SFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:

install:	lorder
	cp lorder $(BINDIR)/$(SGS)lorder

clobber:
	-rm -f lorder

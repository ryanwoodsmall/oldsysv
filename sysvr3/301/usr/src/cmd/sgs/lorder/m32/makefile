#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lorder:m32/makefile	1.8"
#
#	BELLMAC-32 LORDER MAKEFILE
#
#
SGS=m32
OWN=bin
GRP=bin
SED=/bin/sed
SEDFLAGS=-e
SED_CMD=$(SED) $(SEDFLAGS)
#
#	Internal Directories
#
BASE= ../..
BINDIR= $(BASE)/xenv/m32/bin
COMLORDER= $(BASE)/lorder/common
#
all:		lorder
#
lorder:		$(COMLORDER)/lorder
		-rm -f lorder
		$(SED_CMD) "s/SGS/$(SGS)/" < $(COMLORDER)/lorder > lorder
#
install:	$(BINDIR)/$(SGS)lorder
#
$(BINDIR)/$(SGS)lorder:	lorder
		sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)lorder lorder
#
save:		$(BINDIR)/$(SGS)lorder
		-rm -f $(BINDIR)/$(SGS)lorder.back
		cp $(BINDIR)/$(SGS)lorder $(BINDIR)/$(SGS)lorder.back
#
uninstall:	$(BINDIR)/$(SGS)lorder.back
		-rm -f $(BINDIR)/$(SGS)lorder
		cp $(BINDIR)/$(SGS)lorder.back $(BINDIR)/$(SGS)lorder
#
shrink:		
		-rm -f lorder

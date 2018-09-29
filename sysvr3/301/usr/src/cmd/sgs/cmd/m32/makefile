#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sgs-cmd:m32/makefile	1.12"
#
#	BELLMAC-32 CC MAKEFILE
#
#
SGS=m32
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O $(FFLAG)
LDLIBS =
ARCH=AR32WR
DEFLIST=-D$(ARCH) -DM32
INCLIST=-I$(M32INC) -I$(USRINC)
ENV=
FFLAG=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
STRIP=strip
SPFLAGS=
STRIP_CMD=$(STRIP) $(SPFLAGS)
#
LINT=lint
LINTFLAGS=-p
LINT_CMD=$(LINT) $(LINTFLAGS)
#
#	External Directories
#
ROOT=
VERSION=
USRINC= $(ROOT)/usr/$(VERSION)include
#
#	Internal Directories
#
BASE= ../..
BINDIR= $(BASE)/xenv/m32/bin
LIBDIR= $(BASE)/xenv/m32/lib
M32INC= $(BASE)/inc/m32
COMCMD= $(BASE)/cmd/common
#
all:		$(SGS)cc
#
$(SGS)cc:	$(M32INC)/paths.h $(M32INC)/sgs.h $(COMCMD)/cc.c
	$(CC_CMD) -i -o $(SGS)cc $(COMCMD)/cc.c $(LDLIBS)
#
save:		$(BINDIR)/$(SGS)cc
	-rm -f $(BINDIR)/$(SGS)cc.back
	cp $(BINDIR)/$(SGS)cc $(BINDIR)/$(SGS)cc.back
#
install:	$(BINDIR)/$(SGS)cc $(BINDIR)/gencc
#
$(BINDIR)/$(SGS)cc:	$(SGS)cc
	cp $(SGS)cc $(SGS)cc.bak
	-$(STRIP_CMD) $(SGS)cc
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)cc $(SGS)cc
	mv $(SGS)cc.bak $(SGS)cc
#
$(BINDIR)/gencc:	$(COMCMD)/gencc
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/gencc $(COMCMD)/gencc
#
uninstall:	$(BINDIR)/$(SGS)cc.back
	-rm -f $(BINDIR)/$(SGS)cc
	cp $(BINDIR)/$(SGS)cc.back $(BINDIR)/$(SGS)cc
#
shrink:
	-rm -f $(SGS)cc
#
lint:		$(M32INC)/paths.h $(M32INC)/sgs.h $(COMCMD)/cc.c
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMCMD)/cc.c

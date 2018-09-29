#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)as:m32/makefile	1.24"
#
#	WE32000 AS MASTER MAKEFILE
#
# Architectures:
#
#	AR16WR = DEC 11/45-70
#	AR32WR = VAX 11/780
#	AR32W  = 3B family, IBM 370
#
MAKE=make
SGS=m32
OWN=bin
GRP=bin
CC=cc 
CFLAGS=-O -c
ARCH=AR32W
LDFLAGS=
LDLIBS=
B30=
DBO=FBO
#	Set MAC to ABWRMAC if it is 32A,32B SGS and RESTORE fix is needed
#	Set MAC to ABWORMAC if it is 32A,32B SGS and RESTORE fix is not needed
#	Set MAC to BMAC if it is 32B SGS
MAC=ABWRMAC
PCCALL=CALLPCREL
FLEX=-DFLEXNAMES
#
CCPARMS=CC="$(CC)" CFLAGS="$(CFLAGS)" B30="$(B30)" FFLAG="$(FFLAG)" \
	 ENV="$(ENV)" VERSION="$(VERSION)" USRINC="$(USRINC)"
SGSPARMS=MAKE="$(MAKE)" SGS="$(SGS)" ARCH="$(ARCH)" DBO=$(DBO) \
	 FLEX="$(FLEX)" MAC="$(MAC)"
DIRPARMS=BINDIR="$(BINDIR)" LIBDIR="$(LIBDIR)" ROOT="$(ROOT)"
#
#
#	Internal Directories
#
BASE= ../..
BINDIR= $(BASE)/xenv/m32/bin
LIBDIR= $(BASE)/xenv/m32/lib
ROOT=
VERSION=
USRINC=$(ROOT)/usr/$(VERSION)include
#
all	:	threeproc.mk oneproc.mk
	@if [ $(ARCH) = "AR32W" ] || [ $(ARCH) = "AR32WR" ]; then \
	   $(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) LDFLAGS="$(LDFLAGS)" LDLIBS="$(LDLIBS)" -f oneproc.mk; \
	else \
	   $(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) LDLIBS="$(LDLIBS)" -f threeproc.mk; \
	fi;
#
as1:	threeproc.mk
	@if [ $(ARCH) = "AR16WR" ]; then \
		$(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) -f threeproc.mk $(SGS)as1; \
	else \
		echo "Can't $(MAKE) as1 in one process as"; \
	fi;
#
as2:	threeproc.mk
	@if [ $(ARCH) = "AR16WR" ]; then \
		$(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) -f threeproc.mk as2; \
	else \
		echo "Can't $(MAKE) as2 in one process as"; \
	fi;
#
install	:	threeproc.mk oneproc.mk
	@if [ $(ARCH) = "AR32W" ] || [ $(ARCH) = "AR32WR" ]; then \
		$(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) OWN=$(OWN) GRP=$(GRP) -f oneproc.mk install; \
	else \
		$(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) -f threeproc.mk install; \
	fi;
#
save	:	threeproc.mk oneproc.mk
	@if [ $(ARCH) = "AR32W" ] || [ $(ARCH) = "AR32WR" ]; then \
		$(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) -f oneproc.mk save; \
	else \
		$(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) -f threeproc.mk save; \
	fi;
#
uninstall:	threeproc.mk oneproc.mk
	@if [ $(ARCH) = "AR32W" ] || [ $(ARCH) = "AR32WR" ]; then \
		$(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) -f oneproc.mk uninstall; \
	else \
		$(MAKE) $(CCPARMS) $(SGSPARMS) $(DIRPARMS) -f threeproc.mk uninstall; \
	fi;
#
shrink	:	threeproc.mk oneproc.mk
	@if [ $(ARCH) = "AR32W" ] || [ $(ARCH) = "AR32WR" ]; then \
		$(MAKE) $(SGSPARMS) -f oneproc.mk shrink; \
	else \
		$(MAKE) $(SGSPARMS) -f threeproc.mk shrink; \
	fi;

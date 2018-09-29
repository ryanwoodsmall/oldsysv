#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
#	WE3200 OPTIMIZER MAKEFILE
#
#	@(#)f77optim:om32/f77optim.mk	1.5.1.1
#
SGS=m32
CC=cc
#
CFLAGS=-O
ARCH=AR32WR
DEFLIST=-D$(ARCH) -DM32
INCLIST=-I$(COMOPT) -I$(M32INC) -I$(M32OPT) -I$(USRINC)
ENV=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
LDFLAGS=
LD_CMD=$(CC) $(LDFLAGS)
#
STRIP=strip
SPFLAGS=
STRIP_CMD=$(STRIP) $(SPFLAGS)
#
LINT=lint
LINTFLAGS=-p
LINT_CMD=$(LINT) $(LINTFLAGS)
ROOT=
INSTALL = install
VERSION=
#
#	Internal Directories
#
BASE= ../..
BINDIR= $(BASE)/xenv/m32/bin
LIBDIR= $(ROOT)/usr/lib
M32INC= $(BASE)/optim/om32
COMOPT= $(BASE)/optim/common
M32OPT= $(BASE)/optim/3b
USRINC=$(ROOT)/usr/$(VERSION)include
#
OBJECTS= optim.o inter.o local.o \
		optutil.o peep.o w1opt.o w2opt.o w3opt.o
#
all:		f77optim
#
f77optim:	$(OBJECTS)
		$(LD_CMD) -o f77optim $(OBJECTS)
#
optim.o:	$(COMOPT)/optim.h $(M32OPT)/defs $(COMOPT)/optim.c
		$(CC_CMD) -c $(COMOPT)/optim.c
#
inter.o:	$(COMOPT)/optim.h $(M32OPT)/defs $(COMOPT)/inter.c
		$(CC_CMD) -c $(COMOPT)/inter.c
#
local.o:	$(M32INC)/sgs.h $(M32INC)/paths.h $(COMOPT)/optim.h $(M32OPT)/defs $(M32OPT)/local.c
		$(CC_CMD) -c $(M32OPT)/local.c
#
optutil.o:	$(M32OPT)/defs $(COMOPT)/optim.h $(M32OPT)/optutil.c
		$(CC_CMD) -c $(M32OPT)/optutil.c
#
peep.o:		$(M32OPT)/defs $(COMOPT)/optim.h $(M32OPT)/peep.c
		$(CC_CMD) -c $(M32OPT)/peep.c
#
w1opt.o:	$(M32OPT)/optutil.h $(M32OPT)/defs $(COMOPT)/optim.h $(M32OPT)/w1opt.c
		$(CC_CMD) -c $(M32OPT)/w1opt.c
#
w2opt.o:	$(M32OPT)/optutil.h $(M32OPT)/defs $(COMOPT)/optim.h $(M32OPT)/w2opt.c
		$(CC_CMD) -c $(M32OPT)/w2opt.c
#
w3opt.o:	$(M32OPT)/optutil.h $(M32OPT)/defs $(COMOPT)/optim.h $(M32OPT)/w3opt.c
		$(CC_CMD) -c $(M32OPT)/w3opt.c
#
install:	$(LIBDIR)/f77optim
#
$(LIBDIR)/f77optim:	f77optim
		$(INSTALL) -f $(LIBDIR) f77optim
		-$(STRIP_CMD) $(LIBDIR)/f77optim
#
save:		$(LIBDIR)/f77optim
		-rm -f $(LIBDIR)/f77optim.back
		cp $(LIBDIR)/f77optim $(LIBDIR)/f77optim.back
#
uninstall:	$(LIBDIR)/f77optim.back
		-rm -f $(LIBDIR)/f77optim
		cp $(LIBDIR)/f77optim.back $(LIBDIR)/f77optim
#
clobber:
		-rm -f $(OBJECTS) f77optim
#
clean:
		-rm -f $(OBJECTS)

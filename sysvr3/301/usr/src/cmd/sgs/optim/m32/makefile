#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)optim:m32/makefile	1.13"
#
#	WE32000 OPTIMIZER MAKEFILE
#
#
SGS=m32
CC=cc
#
OWN=bin
GRP=bin
#
CFLAGS=-c -O
ARCH=AR32W
DEFLIST=-D$(ARCH) -DM32
INCLIST= -I. -I$(COMOPT) -I$(M32INC) -I$(USRINC)
ENV=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
LDFLAGS=
LDLIBS=
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
VERSION=
USRINC= $(ROOT)/usr/$(VERSION)include
#
#	Internal Directories
#
BASE= ../..
BINDIR= $(BASE)/xenv/m32/bin
LIBDIR= $(BASE)/xenv/m32/lib
M32INC= $(BASE)/inc/m32
COMOPT= $(BASE)/optim/common
#
OBJECTS= optim.o inter.o local.o \
		optutil.o peep.o w1opt.o w2opt.o w3opt.o func.o
#
all:		optim
#
optim:		$(OBJECTS)
		$(LD_CMD) -o optim $(OBJECTS) $(LDLIBS)
#
optim.o:	$(COMOPT)/optim.h defs $(COMOPT)/optim.c
		$(CC_CMD) $(COMOPT)/optim.c
#
inter.o:	$(COMOPT)/optim.h defs $(COMOPT)/inter.c
		$(CC_CMD) $(COMOPT)/inter.c
#
local.o:	$(M32INC)/sgs.h $(M32INC)/paths.h $(COMOPT)/optim.h defs local.c
		$(CC_CMD) local.c
#
optutil.o:	defs $(COMOPT)/optim.h optutil.c
		$(CC_CMD) optutil.c
#
peep.o:		defs $(COMOPT)/optim.h peep.c
		$(CC_CMD) peep.c
#
w1opt.o:	optutil.h defs $(COMOPT)/optim.h w1opt.c
		$(CC_CMD) w1opt.c
#
w2opt.o:	optutil.h defs $(COMOPT)/optim.h w2opt.c
		$(CC_CMD) w2opt.c
#
w3opt.o:	optutil.h defs $(COMOPT)/optim.h w3opt.c
		$(CC_CMD) w3opt.c
#
func.o:		optutil.h defs $(COMOPT)/optim.h func.c
		$(CC_CMD) func.c
#
install:	$(LIBDIR)/$(SGS)optim
#
$(LIBDIR)/$(SGS)optim:	optim
		cp optim optim.bak
		-$(STRIP_CMD) optim
		sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(LIBDIR)/$(SGS)optim optim
		mv optim.bak optim
#
save:		$(LIBDIR)/$(SGS)optim
		-rm -f $(LIBDIR)/$(SGS)optim.back
		cp $(LIBDIR)/$(SGS)optim $(LIBDIR)/$(SGS)optim.back
#
uninstall:	$(LIBDIR)/$(SGS)optim.back
		-rm -f $(LIBDIR)/$(SGS)optim
		cp $(LIBDIR)/$(SGS)optim.back $(LIBDIR)/optim
#
shrink:		clobber
clobber:	clean
		-rm -f optim
clean:
		-rm -f $(OBJECTS)

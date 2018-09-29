#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)stincc:m32/makefile	1.13"
#
#
SGS=	m32
OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	775		#protections
BASE=	../..
ROOT=	$(BASE)
VERSION=

# compiler version string to be output to assembler
COMPVERS=	"\"02.01\""

USRINC= $(ROOT)/usr/$(VERSION)include

BINDIR=	$(ROOT)/bin
LIBDIR=	$(ROOT)/lib
LDLIBS =
COMINC=	.
PCC2COM= ../common
STINCCOM= ../scommon
STYFLAGS=
# Directory for sdb.h
SDBINC=	.
# Directory for paths.h
M32INC= ../../inc/m32

FLEX=	-DFLEXNAMES

CC=	cc
LINT=	lint
YACC=	yacc
LFLAGS=	-p
CFLAGS=	-O
INCLIST=	-I. -I $(PCC2COM) -I$(SDBINC) -I$(M32INC) -I$(USRINC)
#
#	To compile with enhanced asm capability, compile with
#		ASM	= -DIN_LINE
#		INLINE.C= $(PCC2COM)/inline.c
#		INLINE.O= inline.o
#	Otherwise, set all to null
ASM	= -DIN_LINE
INLINE.C= $(PCC2COM)/inline.c
INLINE.O= inline.o
#
DEFLIST= -DM32B -DNODBG -DNOSIMPSTR -DSTINCC $(ASM)
ODEFLIST=	$(FLEX)
LDFLAGS=
YFLAGS=	-l
#
STRIP=strip
SPFLAGS=
STRIP_CMD=$(STRIP) $(SPFLAGS)
CC_CMD=	$(CC) -c $(CFLAGS) $(INCLIST) $(DEFLIST) $(ODEFLIST)
YACC_CMD=	$(YACC) $(YFLAGS)

# use M as a short-hand for the awkward PCC2COM, S for STINCCOM
# (M for Machine independent part, S for Stincc code)

M=	$(PCC2COM)
S=	$(STINCCOM)


OFILES= cgram.o xdefs.o scan.o pftn.o trees.o optim.o local.o reader.o\
	local2.o debug.o match.o allo.o comm1.o table.o cost.o cgen.o\
	optdir.o fparith.o atof2.o $(INLINE.O)
CFILES=	cgram.c $M/xdefs.c $M/scan.c $M/pftn.c $M/cgen.c $S/cost.c \
	$M/trees.c $M/optim.c local.c $M/reader.c \
	local2.c $M/debug.c $S/match.c $M/allo.c $M/comm1.c \
	table.c optdir.c fparith.c atof2.c $(INLINE.C)

build:	comp

#-------------------------

comp:	$(OFILES)
		$(CC) $(CFLAGS) $(LDFLAGS) -o comp $(OFILES) $(LDLIBS)

sty:		$M/manifest.h macdefs.h $M/mfile2.h $M/dope.h sty.c
		cc $(ASM) $(CFLAGS) $(LDFLAGS) -I. -I$M -o sty sty.c  #native cc !!!
sty.c:		$S/sty.y
		-@echo "Expect 3 reduce/reduce conflicts"
		$(YACC_CMD) $S/sty.y
		mv y.tab.c sty.c

trees.o:	$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $M/trees.c
		$(CC_CMD) $M/trees.c

optim.o:	$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $M/optim.c
		$(CC_CMD) $M/optim.c

pftn.o:		$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $M/pftn.c
		$(CC_CMD) $M/pftn.c

local.o:	$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $(M32INC)/paths.h
		$(CC_CMD) -DCOMPVERS=$(COMPVERS) local.c

scan.o:		$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $M/scan.c
		$(CC_CMD) $M/scan.c

xdefs.o:	$M/manifest.h $M/mfile1.h $(SDBINC)/sdb.h macdefs.h $M/xdefs.c
		$(CC_CMD) $M/xdefs.c

cgram.o:	$M/manifest.h $M/mfile1.h $(SDBINC)/sdb.h macdefs.h cgram.c
		$(CC_CMD) -DYYDEBUG=0 cgram.c

cgram.c:	$M/cgram.y
		-@echo "Expect 9 shift/reduce conflicts"
		$(YACC_CMD) $M/cgram.y
		mv y.tab.c cgram.c

COMMON=		$M/common $M/dope.h
comm1.o:	$M/manifest.h $M/mfile1.h $(SDBINC)/sdb.h $(COMMON) macdefs.h \
			$M/comm1.c
		$(CC_CMD) $M/comm1.c

table.o:	$M/manifest.h $M/mfile2.h macdefs.h table.c
		$(CC_CMD) table.c

table.c:	sty stin
		-@echo "Expect many \"hides\" and \"out of registers\" messages"
		./sty $(STYFLAGS) stin table.c

cost.o:		$M/manifest.h $M/mfile2.h macdefs.h $S/cost.c
		$(CC_CMD) $S/cost.c

cgen.o:		$M/manifest.h $M/mfile2.h macdefs.h $M/cgen.c
		$(CC_CMD) $M/cgen.c

reader.o:	$M/manifest.h $M/mfile2.h macdefs.h $M/reader.c
		$(CC_CMD) $M/reader.c

local2.o:	$M/manifest.h $M/mfile2.h macdefs.h
		$(CC_CMD) local2.c

debug.o:	$M/mfile1.h macdefs.h $(COMINC)/storclass.h $M/debug.c
		$(CC_CMD) $M/debug.c

match.o:	$M/manifest.h $M/mfile2.h macdefs.h $S/match.c $(SDBINC)/sdb.h
		$(CC_CMD) $S/match.c

allo.o:		$M/manifest.h $M/mfile2.h macdefs.h $M/allo.c
		$(CC_CMD) $M/allo.c

optdir.o:	$M/mfile1.h macdefs.h $M/manifest.h $(COMINC)/storclass.h
		$(CC_CMD) optdir.c

inline.o:	$M/mfile2.h macdefs.h $M/manifest.h $M/inline.c
		$(CC_CMD) $M/inline.c

atof2.o:
		$(CC_CMD) atof2.c

fparith.o:	fp.h
		$(CC_CMD) fparith.c

#-------------------------

install:	comp
		cp comp comp.bak
		-$(STRIP_CMD) comp
		sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(LIBDIR)/$(SGS)comp comp
		mv comp.bak comp

#--------------------------

save:	$(LIBDIR)/comp
	-rm -f $(LIBDIR)/comp
	cp $(LIBDIR)/comp $(LIBDIR)/comp.back

#--------------------------

uninstall:	$(LIBDIR)/comp.back
		-rm -f $(LIBDIR)/comp
		cp $(LIBDIR)/comp.back $(LIBDIR)/comp

#--------------------------

shrink:	clobber

clean:
	-rm -f $(OFILES)

#--------------------------

clobber:	clean
		-rm -f comp table.c sty cgram.c sty.c

#--------------------------

lint:	$(CFILES)
	$(LINT) $(LFLAGS) $(INCLIST) $(DEFLIST) $(ODEFLIST) $(CFILES) >lint.out
#	$(LINT) $(LFLAGS) $(INCLIST) $(DEFLIST) $(ODEFLIST) sty.c >sty.out

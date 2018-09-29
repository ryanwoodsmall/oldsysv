#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xenv:m32/template.mk	1.35"
#
#	WE-32000 GLOBAL MAKEFILE
#
#
#	SGS indicates the prefix to be applied to the installed commands.
SGS=m32
#
#	ARCH indicates the architecture of the host machine
#		AR16WR=11/70, AR32WR=vax, AR32W=ibm, 3b20, etc
ARCH=AR32W
#
#	The following macros define the various installation directories.
#	Note that if relative pathnames are used, they must allow for the
#	"cd" issued prior to each make.
#
#	BASE is the directory under which all the sgs source lives
BASE= ../..
#
#	BINDIR is the directory for installing executable ("bin") files.
#	It should correspond to the same directory as specified in AS and LD
#	in paths.h.
BINDIR=
#
#	LIBDIR is the directory for installing libraries and executable
#	files not intended for direct user execution (e.g., assembler pass1).
#	It should correspond to the same directory as specified in COMP,
#	OPTIM, AS1, and AS2 in paths.h.
LIBDIR=
#
#	INCDIR is the directory for installing include files used in
#	developing the SGS and which are needed by certain users of the
#	SGS (e.g., operating system developers).  It should correspond to
#	the same directory as specified in INCDIR in paths.h.
INCDIR=
#
#	Specify the byte order for this SGS instance.
#		FBO = Forward Byte Order (3B20, IBM)
#		RBO = Reverse Byte Order (DEC)
DBO=FBO
#
#	If host machine is running a pre-5.0 release of UNIX
#	then set Archive Format to "OLDAR".
#	Starting with the 5.0 UNIX (SVR1)  release the Archive Format
#	should be set to "PORTAR".
#	If building a 5.0 release on a host that is running
#	System V Rel. 2.0 (SVR2), then set ARFORMAT to PORT5AR.
#
ARFORMAT=PORTAR
#
#	Starting with the SVR2 release of UNIX,
#	if flexnames feature is desired, then set FLEX
#	to -DFLEXNAMES.
#	If host machine is running a pre-SVR2 release of UNIX
#	then set FLEX to null (ie. FLEX= ).
#
FLEX=-DFLEXNAMES
#
#	MAC parameter specifies the set of software workarounds
#	to be produced by the new sgs.
#	MAC=ABWORMAC produces sgs for WE32001 and WE32100 chips.
#	MAC=BMAC produces sgs for WE32100  o n l y.
#
MAC=
#	This is the machine ID field. The valid values are
#	u3b15, u3b5 or u3b2.
MACH=
#
#	The following parameter specifies the default include directory
#	for cpp. If not defined the value will be ROOT/usr/include.
#
USRINC=
NOUSRINC=NOUSRINC
#
#
OWN=bin
GRP=bin
#
#
MAKE=make
CC=cc
FFLAG=
ENV=
ROOT=
VERSION=
LDLIBS=
LIBLD=$(BASE)/libld/m32/libld.a
#
#
ENVPARMS=MAKE="$(MAKE)" SGS="$(SGS)" ARCH="$(ARCH)" OWN="$(OWN)" GRP="$(GRP)" \
	DBO="$(DBO)" ARFORMAT="$(ARFORMAT)" FLEX="$(FLEX)" LDLIBS="$(LDLIBS)"

CPPARMS=CC="$(CC)" FFLAG="$(FFLAG)" ENV="$(ENV)" ROOT="$(ROOT)" \
	VERSION="$(VERSION)" USRINC="$(USRINC)"

CCPARMS=CC="$(CC)" FFLAG="$(FFLAG)" ENV="$(ENV)" ROOT="$(ROOT)" \
	VERSION="$(VERSION)" USRINC="$(NOUSRINC)"

DIRPARMS=BINDIR="$(BINDIR)" LIBDIR="$(LIBDIR)"
#
#
all:	libs
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) 
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) 
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) $(CPPARMS) PD_SYS=D_unix \
		PD_MACH=D_newmach NEW_MACH="-DPD_MY_MACH=\\\"$(MACH)\\\""
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) MAC=$(MAC)
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS)
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS)
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/unix_conv/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/../make; /bin/make -f make.mk
#
libs:
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
#
first:
	if [ ! -d $(BINDIR) ] ; then mkdir $(BINDIR) ; fi 
	if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi 
	if [ ! -d $(INCDIR) ] ; then mkdir $(INCDIR) ; fi 
#
install: libinstall
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) $(CPPARMS) $(DIRPARMS) install
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) install
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) install
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/unix_conv/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cp $(BASE)/../make/make $(BINDIR)/m32make
	cp $(BASE)/xenv/m32/env-m32 $(BINDIR)/$(SGS)env
	cp $(BASE)/xenv/m32/make.tmp $(BINDIR)/make
	cp $(BASE)/xenv/m32/vax $(BINDIR)
	cp $(BASE)/xenv/m32/u3b $(BINDIR)
	cp $(BASE)/xenv/m32/u370 $(BINDIR)
	cp $(BASE)/xenv/m32/u3b15 $(BINDIR)
	cp $(BASE)/xenv/m32/u3b5 $(BINDIR)
	cp $(BASE)/xenv/m32/u3b2 $(BINDIR)
#
libinstall:
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
#
save:
	cd $(BASE)/inc/m32; $(MAKE) $(ENVPARMS) INCDIR=$(INCDIR) save
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/unix_conv/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) save
	cd $(BASE)/../make; /bin/make -f make.mk $(DIRPARMS) save
#
uninstall:
	cd $(BASE)/inc/m32; $(MAKE) $(ENVPARMS) INCDIR=$(INCDIR) uninstall
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/../make; /bin/make -f make.mk $(DIRPARMS) uninstall
#
shrink: libshrink
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) shrink
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/unix_conv/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) shrink
	cd $(BASE)/../make; /bin/make -f make.mk clobber
#
libshrink:
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) shrink

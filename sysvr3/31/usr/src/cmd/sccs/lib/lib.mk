#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:lib/lib.mk	6.4"
#
#
LIBS=	cassi.a \
	comobj.a \
	mpwlib.a

PRODUCTS = $(LIBS)

INSTR=

all: $(LIBS)
	@echo "SCCS Libraries are up to date."

cassi.a::
	cd cassi; $(MAKE) -e -f cassi.mk all

comobj.a::
	cd comobj; $(MAKE) -e -f comobj.mk all

mpwlib.a::
	cd mpwlib; $(MAKE) -e -f mpwlib.mk all

install:	$(LIBS)
	cd cassi; $(MAKE) -e -f cassi.mk install
	cd comobj; $(MAKE) -e -f comobj.mk install
	cd mpwlib; $(MAKE) -e -f mpwlib.mk install

clean:
	cd cassi; $(MAKE) -e -f cassi.mk clean
	cd comobj; $(MAKE) -e -f comobj.mk clean
	cd mpwlib; $(MAKE) -e -f mpwlib.mk clean

clobber:
	cd cassi; $(MAKE) -e -f cassi.mk clobber
	cd comobj; $(MAKE) -e -f comobj.mk clobber
	cd mpwlib; $(MAKE) -e -f mpwlib.mk clobber

.PRECIOUS:	$(PRODUCTS)

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/dev.d/hp7220.d/Makefile	1.6"
###
### Standard Constants 
###

ROOT     =
LROOT    =
BIN      = $(ROOT)/$(LROOT)/usr/bin/graf
CC       = cc
CFLAGS   = -O $(FFLAG)
FFLAG    =

GRAFMAKE = $(MAKE) ROOT=$(ROOT) LROOT=$(LROOT) BIN=$(BIN)


####### Dependencies

all         :
	cd hpd.d; $(GRAFMAKE)

install     : 
	cd hpd.d; $(GRAFMAKE) install

clobber     :
	cd hpd.d; $(GRAFMAKE) clobber

clean     :
	cd hpd.d; $(GRAFMAKE) clean

listing     :
	cd hpd.d; $(GRAFMAKE) listing

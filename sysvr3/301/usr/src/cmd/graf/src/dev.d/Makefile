#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/dev.d/Makefile	1.6"
###
### Standard Constants 
###

ROOT     =
LROOT    =
BIN      = $(ROOT)/$(LROOT)/usr/bin/graf
CC       = cc
CFLAGS   = -c -O $(FFLAG)

GRAFMAKE = $(MAKE) ROOT=$(ROOT) BIN=$(BIN) LROOT=$(LROOT)


####### Dependencies
all        :
	cd lolib;     $(GRAFMAKE)
	cd uplib;     $(GRAFMAKE)
	cd hp7220.d;  $(GRAFMAKE)
	cd tek4000.d; $(GRAFMAKE)

install    :
	cd lolib;     $(GRAFMAKE)  install
	cd uplib;     $(GRAFMAKE)  install
	cd hp7220.d;  $(GRAFMAKE)  install
	cd tek4000.d; $(GRAFMAKE)  install

clobber    :
	cd hp7220.d;  $(GRAFMAKE)  clobber
	cd lolib;     $(GRAFMAKE)  clobber
	cd tek4000.d; $(GRAFMAKE)  clobber
	cd uplib;     $(GRAFMAKE)  clobber

clean    :
	cd hp7220.d;  $(GRAFMAKE)  clean
	cd lolib;     $(GRAFMAKE)  clean
	cd tek4000.d; $(GRAFMAKE)  clean
	cd uplib;     $(GRAFMAKE)  clean

listings   :
	cd hp7220.d;  $(GRAFMAKE)  listings
	cd lolib;     $(GRAFMAKE)  listings
	cd tek4000.d; $(GRAFMAKE)  listings
	cd uplib;     $(GRAFMAKE)  listings

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:graf.mk	1.11"

###
### Standard Constants that are always explicitly passed through make
###

ROOT     =
LROOT    =
BIN1     = $(ROOT)/$(LROOT)/usr/bin
BIN      = $(ROOT)/$(LROOT)/usr/bin/graf
SRC      = $(ROOT)/$(LROOT)/usr/src/cmd/graf
LIB      = $(ROOT)/$(LROOT)/usr/lib/graf
CC       = cc
CFLAGS   = -c -O $(FFLAG)

###
### Standard constants that are not passed through make
###

GRAFMAKE = $(MAKE) ROOT=$(ROOT) LROOT=$(LROOT) BIN=$(BIN)

###
### Local Constants
###

ARGS =
ARGH =	$(ROOT)/$(LROOT)/usr/src/arglist

### Dependencies

all:
	cd src; $(GRAFMAKE)

install:
	if test ! -d $(BIN); then mkdir $(BIN); chmod 755 $(BIN); fi
	if test ! -d $(SRC)/lib; then mkdir $(SRC)/lib; chmod 755 $(SRC)/lib; fi
	if test ! -d $(LIB); then mkdir $(LIB); chmod 755 $(LIB); fi
	
	if test -x $(ARGH); then                                  \
	  cd src; $(GRAFMAKE) `$(ARGH) + $(ARGS)` install;        \
	else                                                      \
	  cd src; $(GRAFMAKE) install;                            \
	fi
	$(CH)chgrp bin $(BIN1)/graphics; $(CH)chown bin $(BIN1)/graphics
	$(CH)chgrp bin `find $(BIN) -print`;
	$(CH)chown bin `find $(BIN) -print` 
	$(CH)chgrp bin `find $(LIB) -print`; 
	$(CH)chown bin `find $(LIB) -print`

clobber:
	-rm -f ./lib/*
	cd src; $(GRAFMAKE) clobber

clean:
	cd src; $(GRAFMAKE) clean

listings :
	cd src; $(GRAFMAKE) listings

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/Makefile	1.9"
###
### Standard Constants 
###

ROOT     =
LROOT    =
BIN1     = $(ROOT)/$(LROOT)/usr/bin
BIN      = $(ROOT)/$(LROOT)/usr/bin/graf
CC       = cc
CFLAGS   = -c -O $(FFLAG)

GRAFMAKE = $(MAKE) ROOT=$(ROOT) BIN=$(BIN) LROOT=$(LROOT)

###
### Local Constants
###

LOGDIR = $(ROOT)/$(LROOT)/usr/adm
USRLIB = $(ROOT)/$(LROOT)/usr/lib

#### Dependencies

all  : 
	cd glib.d;   $(GRAFMAKE)
	cd gutil.d;  $(GRAFMAKE)
	cd dev.d;    $(GRAFMAKE) 
	cd gtop.d;   $(GRAFMAKE)
	cd ptog.d;   $(GRAFMAKE)
	cd stat.d;   $(GRAFMAKE)
	cd toc.d;    $(GRAFMAKE)
	cd vtoc.d;   $(GRAFMAKE)
	cd whatis.d; $(GRAFMAKE)
	
install  : graphics whatis
	-cp /dev/null $(LOGDIR)/graflog
	-chmod 666 $(LOGDIR)/graflog
	cpset graphics $(BIN1)

	-if test ! -d $(USRLIB)/graf/whatis; then  \
	 mkdir $(USRLIB)/graf/whatis;              \
	 chmod 755 $(USRLIB)/graf/whatis;         \
	fi
	cpset whatis $(BIN)
	
	cd glib.d;   $(GRAFMAKE) install
	cd gutil.d;  $(GRAFMAKE) install
	cd dev.d;    $(GRAFMAKE) install
	cd gtop.d;   $(GRAFMAKE) install
	cd ptog.d;   $(GRAFMAKE) install
	cd stat.d;   $(GRAFMAKE) install
	cd toc.d;    $(GRAFMAKE) install
	cd vtoc.d;   $(GRAFMAKE) install
	cd whatis.d; $(GRAFMAKE) install
	
graphics  :  graphics.sh
	cp graphics.sh graphics
	chmod 555 graphics

whatis    :
	cp whatis.sh whatis
	chmod 555 whatis

########

clobber   :
	-rm -f whatis graphics
	cd dev.d;    $(GRAFMAKE) clobber
	cd glib.d;   $(GRAFMAKE) clobber
	cd gtop.d;   $(GRAFMAKE) clobber
	cd gutil.d;  $(GRAFMAKE) clobber
	cd ptog.d;   $(GRAFMAKE) clobber
	cd stat.d;   $(GRAFMAKE) clobber
	cd toc.d;    $(GRAFMAKE) clobber
	cd vtoc.d;   $(GRAFMAKE) clobber
	cd whatis.d; $(GRAFMAKE) clobber

clean:
	cd dev.d;    $(GRAFMAKE) clean
	cd glib.d;   $(GRAFMAKE) clean
	cd gtop.d;   $(GRAFMAKE) clean
	cd gutil.d;  $(GRAFMAKE) clean
	cd ptog.d;   $(GRAFMAKE) clean
	cd stat.d;   $(GRAFMAKE) clean
	cd toc.d;    $(GRAFMAKE) clean
	cd vtoc.d;   $(GRAFMAKE) clean
	cd whatis.d; $(GRAFMAKE) clean
	
listings  :
	pr -h "   src top level files   " whatis graphics | pr
	cd dev.d;    $(GRAFMAKE) listings
	cd glib.d;   $(GRAFMAKE) listings
	cd gtop.d;   $(GRAFMAKE) listings
	cd gutil.d;  $(GRAFMAKE) listings
	cd ptog.d;   $(GRAFMAKE) listings
	cd stat.d;   $(GRAFMAKE) listings
	cd vtoc.d;   $(GRAFMAKE) listings
	cd toc.d;    $(GRAFMAKE) listings
	cd whatis.d; $(GRAFMAKE) listings

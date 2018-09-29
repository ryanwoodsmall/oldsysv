#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/toc.d/Makefile	1.6"
###
### Standard Constants
###

ROOT     =
LROOT    =
BIN      = $(ROOT)/$(LROOT)/usr/bin/graf
CC       = cc
CFLAGS   = -c -O $(FFLAG)


###
### Local Constants
###

USRLIB   = $(ROOT)/$(LROOT)/usr/lib

##### Dependencies

all:  ttoc dtoc

install     :  dtoc  ttoc
	
	-if test ! -d $(USRLIB)/graf; then  \
	  mkdir $(USRLIB)/graf;            \
	  chmod 755 $(USRLIB)/graf;        \
        fi
	-if test ! -d $(USRLIB)/graf/ttoc.d; then  \
	  mkdir $(USRLIB)/graf/ttoc.d;            \
	  chmod 755 $(USRLIB)/graf/ttoc.d;        \
	fi
	
	cpset ttoc $(BIN)
	cpset ed.notoc $(USRLIB)/graf/ttoc.d
	cpset ed.toc $(USRLIB)/graf/ttoc.d
	cpset ed.ttoc.t $(USRLIB)/graf/ttoc.d
	cpset dtoc $(BIN)

ttoc        :  ttoc.sh
	cp ttoc.sh ttoc
	chmod 555 ttoc

dtoc        :  dtoc.sh
	cp dtoc.sh dtoc
	chmod 555 dtoc

########

clobber     :      ;
	-rm -f dtoc ttoc

clean:

listings    :      ;
	ls -l *.sh | pr -h "    toc  source files " | pr

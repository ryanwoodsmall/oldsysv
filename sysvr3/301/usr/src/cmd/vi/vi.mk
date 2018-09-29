#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vi:vi.mk	1.26"

#	Makefile for vi

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS =

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = vi.mk

VIMAKE = $(MAKE) -f $(MKFILE) ROOT=$(ROOT)  DIR=$(DIR)

MKDIR = port
MKFILE = makefile.usg         

ALL : all

all : 
	@echo "\n\t>Making commands."
	cd misc; ${MAKE} ROOT=$(ROOT) all; cd ..

	if ( u370 || pdp11 ) ;\
	then \
		if [ "x`uname -m`" = "xc70" ] ;\
		then \
			$(MAKE) -f MKFILE=makefile.c70 ROOT=$(ROOT)  DIR=$(DIR) ;\
		elif [ "x`uname -m`" = "xpdp11-70" ] ;\
		then \
			$(MAKE) -f MKFILE=pdp11 ROOT=$(ROOT)  DIR=$(DIR) ;\
		elif [ "x`uname -m`" = "x370" ] ;\
		then \
			$(MAKE) -f MKFILE=makefile.370 ROOT=$(ROOT)  DIR=$(DIR) ;\
		fi \
	fi


	cd $(MKDIR) ; $(VIMAKE) all ; cd ..
	@echo "Finished compiling..."

install: all
	cd misc; $(MAKE) ROOT=$(ROOT) DIR=$(DIR) install ; cd ..;
	-if pdp11; \
	 then \
		cd pdp11 ; ${MAKE} install -f makefile.usg ; cd ..; \
		cd pdp11 ; ${MAKE} installutil -f makefile.usg; cd ..; \
	else \
		echo "\n\t> Installing ex object."; \
		cd $(MKDIR) ; ${VIMAKE} install ; cd ..; \
	fi

size: all
	cd misc ; $(MAKE) ROOT=$(ROOT) DIR=$(DIR)  size ; cd ..
	cd $(MKDIR) ; ${VIMAKE} size ; cd ..

strip: all
	cd misc ; $(MAKE) ROOT=$(ROOT) DIR=$(DIR)  strip ; cd ..
	cd $(MKDIR) ; ${VIMAKE} strip ; cd ..

#
# Cleanup procedures
#
clobber: 
	cd misc ; ${MAKE} ROOT=$(ROOT) clobber ; cd ..
	-if pdp11; \
	then \
		cd pdp11; \
		${MAKE} -f makefile.usg clobber ;\
	else \
		cd port; \
		${VIMAKE} clobber ;\
	fi

clean:
	cd misc ; ${MAKE} ROOT=$(ROOT) clean ; cd ..
	-if pdp11; \
	then \
		cd pdp11; \
		${MAKE} -f makefile.usg clean ;\
	else \
		cd port; \
		${VIMAKE} clean ;\
	fi

#	These targets are useful but optional

partslist:
	cd misc ; ${MAKE} ROOT=$(ROOT) partslist ; cd ..
	cd $(MKDIR) ; ${VIMAKE} partslist ; cd ..

productdir:
	cd misc ; ${MAKE} ROOT=$(ROOT) productdir ; cd ..
	cd $(MKDIR) ; ${VIMAKE} productdir ; cd ..

product:
	cd misc ; ${MAKE} ROOT=$(ROOT) product ; cd ..
	cd $(MKDIR) ; ${VIMAKE} product ; cd ..

srcaudit:
	cd misc ; ${MAKE} ROOT=$(ROOT) srcaudit ; cd ..
	cd $(MKDIR) ; ${VIMAKE} srcaudit ; cd ..

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)assist:assist.mk	1.11"
ROOT =
INSDIR = $(ROOT)/usr/bin
LIB = $(ROOT)/usr/lib
LIBDIR = $(LIB)/assist/lib
ALIBDIR = `echo $(LIBDIR) | sed \"s;$(ROOT);;\"`
BINDIR = $(LIB)/assist/bin
ABINDIR = `echo $(BINDIR) | sed \"s;$(ROOT);;\"`
CURSES = -lcurses
INCLUDE =
MAKE = make
INS = /etc/install
FFLAG =

all:
	cd ./src/forms; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) INCLUDE=$(INCLUDE) CURSES=$(CURSES) -f forms.mk
	cd ./src/scripts; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) INCLUDE=$(INCLUDE) CURSES=$(CURSES) -f mscript.mk
	cd ./src/search; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) -f msearch.mk
	cd ./src/setup; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) -f setup.mk
	cd ./src/astgen; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) INCLUDE=$(INCLUDE) CURSES=$(CURSES) -f tools.mk

install: $(LIBDIR) $(BINDIR)
	cd ./src/forms; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) INCLUDE=$(INCLUDE) CURSES=$(CURSES) -f forms.mk install
	cd ./src/scripts; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) INCLUDE=$(INCLUDE) CURSES=$(CURSES) -f mscript.mk install
	cd ./src/search; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) -f msearch.mk install
	cd ./src/setup; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) -f setup.mk install
	cd ./src/astgen; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) FFLAG=$(FFLAG) INS=$(INS) INCLUDE=$(INCLUDE) CURSES=$(CURSES) -f tools.mk install
	cd ./lib/forms; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f forms.mk install
	cd ./lib/scripts; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f scripts.mk install
	cd ./lib/search; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f search.mk install
	cd ./lib/setup; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f setup.mk install
	cd ./lib/astgen; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f tools.mk install
	$(CH)chmod 755 $(BINDIR); chgrp bin $(BINDIR); chown bin $(BINDIR)
	$(CH)chmod 755 $(LIBDIR); chgrp bin $(LIBDIR); chown bin $(LIBDIR)
########################################################################
#
#	NOTE: The next section creates an edit script
#		to automatically change assist.sh to
#		point to the bin and lib directories
#		and then installs assist.sh in the bin
#		directory as assist.  If the format
#		of assist.sh changes, it is necessary to
#		check the next section for possible changes.
#
########################################################################
	@echo "/^ASSISTLIB" > ./assist.edit
	@echo "s;=.*;=$(ALIBDIR)" >> ./assist.edit
	@echo "/^ASSISTBIN" >> ./assist.edit
	@echo "s;=.*;=$(ABINDIR)" >> ./assist.edit
	@echo "w assist" >> ./assist.edit
	@echo "q" >> ./assist.edit
	ed ./src/assist.sh < ./assist.edit > /dev/null
	@rm ./assist.edit
	$(INS) -f $(INSDIR) assist
########################################################################
#
#	NOTE: The next section creates an edit script
#		to automatically change astgen.sh to
#		point to the bin and lib directories
#		and then installs astgen.sh in the bin
#		directory as astgen.  If the format
#		of astgen.sh changes, it is necessary to
#		check the next section for possible changes.
#
########################################################################
	@echo "/^ASSISTLIB" > ./astgen.edit
	@echo "s;=.*;=$(ALIBDIR)" >> ./astgen.edit
	@echo "/^ASSISTBIN" >> ./astgen.edit
	@echo "s;=.*;=$(ABINDIR)" >> ./astgen.edit
	@echo "w astgen" >> ./astgen.edit
	@echo "q" >> ./astgen.edit
	ed ./src/astgen.sh < ./astgen.edit > /dev/null
	@rm ./astgen.edit
	$(INS) -f $(INSDIR) astgen

clobber:
	cd ./src/forms; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f forms.mk clobber
	cd ./src/scripts; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f mscript.mk clobber
	cd ./src/search; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f msearch.mk clobber
	cd ./src/setup; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f setup.mk clobber
	cd ./src/astgen; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f tools.mk clobber
	-rm -f assist astgen

clean:
	cd ./src/forms; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f forms.mk clean
	cd ./src/scripts; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f mscript.mk clean
	cd ./src/search; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f msearch.mk clean
	cd ./src/setup; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f setup.mk clean
	cd ./src/astgen; $(MAKE) -e LIBDIR=$(LIBDIR) BINDIR=$(BINDIR) -f tools.mk clean

$(BINDIR):
	if [ ! -d `dirname $(BINDIR)` ] ;\
	then \
		mkdir `dirname $(BINDIR)` ;\
	fi
	if [ ! -d $(BINDIR) ] ;\
	then \
		mkdir $(BINDIR) ;\
	fi

$(LIBDIR):
	if [ ! -d `dirname $(LIBDIR)` ] ;\
	then \
		mkdir `dirname $(LIBDIR)` ;\
	fi
	if [ ! -d $(LIBDIR) ] ;\
	then \
		mkdir $(LIBDIR) ;\
	fi

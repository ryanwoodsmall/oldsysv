#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)terminfo:terminfo.mk	1.2"
#
#	terminfo makefile
#

ROOT=
TERMDIR=${ROOT}/usr/lib/terminfo
TABDIR=${ROOT}/usr/lib/tabset
PARTS=	header *.ti trailer
COMPILE=tic -v

all:	ckdir terminfo.src
	TERMINFO=${TERMDIR} 2>&1 $(COMPILE) terminfo.src > errs
	@touch install
	@echo
	@sh ./ckout
	@echo
	@echo
	@echo
	cp tabset/* $(TABDIR)

install: all

terminfo.src:	$(PARTS)
	@cat $(PARTS) > terminfo.src

clean:
	rm -f terminfo.src install errs nohup.out

clobber: clean

ckdir:
	@echo "terminfo database will be built in $(TERMDIR)."
	@echo "checking for the existence of $(TERMDIR):"
	@echo
	@if [ -d $(TERMDIR) ]; \
	then \
		echo "\t$(TERMDIR) exists"; \
	else  \
		echo "\tbuilding $(TERMDIR)"; \
		mkdir $(TERMDIR); \
		chown bin $(TERMDIR); \
		chgrp bin $(TERMDIR); \
		chmod 775 $(TERMDIR); \
	fi
	@echo
	@echo
	@echo
	@echo "terminfo database will reference file in $(TABDIR)."
	@echo "checking for the existence of $(TABDIR):"
	@echo
	@if [ -d $(TABDIR) ]; \
	then \
		echo "\t$(TABDIR) exists"; \
	else  \
		echo "\tbuilding $(TABDIR)"; \
		mkdir $(TABDIR); \
		chown bin $(TABDIR); \
		chgrp bin $(TABDIR); \
		chmod 775 $(TABDIR); \
	fi
	@echo
	@echo It will take quite some time to generate $(TERMDIR)/*/*.
	@echo

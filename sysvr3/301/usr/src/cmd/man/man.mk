#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)man:man.mk	1.6"
#	makefile for man command.

OL = $(ROOT)/
INS = :
INSDIR = $(OL)usr/bin
MAKE = make

all:  man

man:	man.sh
	cp man.sh man
	$(INS) man $(INSDIR)
	cd $(INSDIR); chmod 755 man; $(CH) chgrp bin man; chown bin man

install:
	$(MAKE) -f man.mk INS=cp ROOT=$(ROOT) CH=$(CH)

clean:

clobber:  clean
	rm -f man

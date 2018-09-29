#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)setup.files:setup.mk	1.7"

LIBDIR = ..
CH =

FILES = assistrc messages t.altchar t.basic t.fkeys t.standout

install :
	-mkdir $(LIBDIR)/setup
	-cp $(FILES) $(LIBDIR)/setup
	$(CH)cd $(LIBDIR)/setup; chmod 444 $(FILES); chgrp bin $(FILES); chown bin $(FILES)
	$(CH)chmod 755 $(LIBDIR)/setup; chgrp bin $(LIBDIR)/setup; chown bin $(LIBDIR)/setup

clobber :
	:

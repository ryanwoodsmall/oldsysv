#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)search.files:search.mk	1.8"

LIBDIR = ..
CH =

FILES = searchlist
UFILES = unix/matchpairs unix/search-file

install :
	-mkdir $(LIBDIR)/search $(LIBDIR)/search/unix
	-cp $(FILES) $(LIBDIR)/search
	-cp $(UFILES) $(LIBDIR)/search/unix
	$(CH)cd $(LIBDIR)/search; chmod 444 $(FILES) $(UFILES); chgrp bin $(FILES); chown bin $(FILES); chgrp bin $(UFILES); chown bin $(UFILES)
	$(CH)chmod 755 $(LIBDIR)/search/unix $(LIBDIR)/search
	$(CH)chgrp bin $(LIBDIR)/search/unix $(LIBDIR)/search
	$(CH)chown bin $(LIBDIR)/search/unix $(LIBDIR)/search

clobber :
	:

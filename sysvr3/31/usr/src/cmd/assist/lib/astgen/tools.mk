#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)tools.files:tools.mk	1.10"

LIBDIR = ..
CH =

FILES = \
cf_f.help cf_g.help cfinfo ftype.help m_f.help \
m_g.help menuinfo perm.help top.help vals.help

install : $(FILES)
	-mkdir $(LIBDIR)/astgen
	-cp $(FILES) $(LIBDIR)/astgen
	$(CH)cd $(LIBDIR)/astgen; chmod 444 $(FILES); chgrp bin $(FILES); chown bin $(FILES)
	$(CH)chmod 755 $(LIBDIR)/astgen; chgrp bin $(LIBDIR)/astgen; chown bin $(LIBDIR)/astgen

clobber :
	-rm -f $(FILES)

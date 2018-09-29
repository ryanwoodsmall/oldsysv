#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)script.files:scripts.mk	1.10"

LIBDIR = ..
CH =

FILES = \
as.build sdb.build t.ascf t.ascontinue t.asending t.ashelp t.asintro \
t.asmenu t.aspopmenu t.assubs t.sdbbreak t.sdbcontinue t.sdbcore t.sdbending \
t.sdbexec \
t.sdbintro t.sdbmulti t.sdbsdbend t.sdbsearch t.sdbsubs t.viadd t.vicontinue \
t.vicursor t.videlete t.videmo t.viending t.viintro t.vimanip t.visubs \
t.viundo t.viintroend t.viopts vi.build vicommands \
un.build t.uncontinue t.unintro t.unrediri t.unsubs t.unending \
t.unintroend t.unrediro t.unfiles t.unpipes t.unstdio

PFILES = \
t.ascf t.ashelp t.asmenu t.aspopmenu t.assubs t.sdbbreak \
t.sdbcore t.sdbexec t.sdbintro t.sdbmulti t.sdbsearch \
t.sdbsubs t.viadd t.vicursor t.videlete t.videmo t.vimanip \
t.viopts t.visubs t.viundo \
t.uncontinue t.unintro t.unrediri t.unsubs t.unending \
t.unintroend t.unrediro t.unfiles t.unpipes t.unstdio


install : $(FILES)
	-mkdir $(LIBDIR)/scripts
	-cp $(FILES) $(LIBDIR)/scripts
	cd $(LIBDIR)/scripts; \
	$(CH)chmod 444 $(FILES); chgrp bin $(FILES); chown bin $(FILES); \
	rm -f *.z ; pack -f $(PFILES)
	$(CH)chmod 755 $(LIBDIR)/scripts; chgrp bin $(LIBDIR)/scripts; chown bin $(LIBDIR)/scripts

clobber :
	:

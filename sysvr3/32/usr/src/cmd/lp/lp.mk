#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:lp.mk	1.6"
#
# Top level makefile for the LP Print Service component
#


USRSPOOL=	$(ROOT)/usr/spool

CMDDIR	=	./cmd
LIBDIR	=	./lib
INCDIR	=	./include
ETCDIR	=	./etc
MODELDIR=	./model
FILTDIR	=	./filter
TERMINFODIR=	./terminfo



FIFODIR	=	$(USRSPOOL)/lp/fifos
FIFOPRI	=	$(FIFODIR)/private
FIFOPUB	=	$(FIFODIR)/public

DIRS	= \
		$(USRSPOOL)/lp \
		$(USRSPOOL)/lp/admins \
		$(USRSPOOL)/lp/admins/lp \
		$(USRSPOOL)/lp/admins/lp/classes \
		$(USRSPOOL)/lp/admins/lp/forms \
		$(USRSPOOL)/lp/admins/lp/interfaces \
		$(USRSPOOL)/lp/admins/lp/logs \
		$(USRSPOOL)/lp/admins/lp/printers \
		$(USRSPOOL)/lp/admins/lp/pwheels \
		$(USRSPOOL)/lp/bin \
		$(USRSPOOL)/lp/logs \
		$(USRSPOOL)/lp/model \
		$(USRSPOOL)/lp/requests \
		$(USRSPOOL)/lp/system \
		$(USRSPOOL)/lp/temp \
		$(FIFODIR) \
		$(FIFOPRI) \
		$(FIFOPUB)

OWNER	=	lp
GROUP	=	sys
DMODES	=	0775
PRIMODE	=	0771
PUBMODE	=	0773

all:		libs \
		cmds \
		etcs \
		models \
		filters \
		terminfos
		
install:	all \
		strip \
		dirs
	cd $(CMDDIR); $(MAKE) install
	cd $(ETCDIR); $(MAKE) install
	cd $(MODELDIR); $(MAKE) install
	cd $(FILTDIR); $(MAKE) install
	cd $(TERMINFODIR); $(MAKE) install

dirs:
	for d in $(DIRS); do if [ ! -d $$d ]; then mkdir $$d; fi; done
	$(CH)chown $(OWNER) $(DIRS)
	$(CH)chgrp $(GROUP) $(DIRS)
	$(CH)chmod $(DMODES) $(DIRS)
	$(CH)chmod $(PRIMODE) $(FIFOPRI)
	$(CH)chmod $(PUBMODE) $(FIFOPUB)

clean:	
	cd $(LIBDIR); $(MAKE) clean
	cd $(CMDDIR); $(MAKE) clean
	cd $(MODELDIR); $(MAKE) clean
	cd $(FILTDIR); $(MAKE) clean
	cd $(TERMINFODIR); $(MAKE) clean

clobber:
	cd $(LIBDIR); $(MAKE) clobber
	cd $(CMDDIR); $(MAKE) clobber
	cd $(MODELDIR); $(MAKE) clobber
	cd $(FILTDIR); $(MAKE) clobber
	cd $(TERMINFODIR); $(MAKE) clobber

strip:
	if [ -n "$(STRIP)" ]; \
	then \
		$(MAKE) STRIP=$(STRIP) -f lp.mk realstrip; \
	else \
		$(MAKE) STRIP=strip -f lp.mk realstrip; \
	fi

realstrip:
	cd $(CMDDIR); $(MAKE) STRIP=$(STRIP) strip
	cd $(MODELDIR); $(MAKE) STRIP=$(STRIP) strip
	cd $(FILTDIR); $(MAKE) STRIP=$(STRIP) strip

libs:
	cd $(LIBDIR); $(MAKE) FUNCDCL="$(FUNCDCL)"

cmds:
	cd $(CMDDIR); $(MAKE)

etcs:
	cd $(ETCDIR); $(MAKE)

models:
	cd $(MODELDIR); $(MAKE)

filters:
	cd $(FILTDIR); $(MAKE)

terminfos:
	cd $(TERMINFODIR); $(MAKE)


prof:
	cd $(LIBDIR); $(MAKE) prof
	cd $(CMDDIR); $(MAKE) prof

lprof:
	cd $(LIBDIR); $(MAKE) lprof
	cd $(CMDDIR); $(MAKE) lprof


lint:
	cd $(LIBDIR); $(MAKE) FUNCDCL="$(FUNCDCL)" lint
	cd $(CMDDIR); $(MAKE) lint
	cd $(MODELDIR); $(MAKE) lint
	cd $(FILTDIR); $(MAKE) lint

lintsrc:
	cd $(LIBDIR); $(MAKE) FUNCDCL="$(FUNCDCL)" lintsrc

lintlib:
	cd $(LIBDIR); $(MAKE) FUNCDCL="$(FUNCDCL)" lintlib


grab:
	( \
		ls ./lp.mk; \
		ls $(INCDIR)/*.h; \
		ls $(CMDDIR)/Makefile; \
		ls $(CMDDIR)/*.[ch]; \
		ls $(CMDDIR)/*/Makefile; \
		ls $(CMDDIR)/*/*.[ch]; \
		ls $(LIBDIR)/Makefile; \
		ls $(LIBDIR)/*/Makefile; \
		ls $(LIBDIR)/*/*.[ch]; \
		ls $(LIBDIR)/*/llib-*; \
		ls $(LIBDIR)/oam/msg.source; \
		ls $(MODELDIR)/*; \
		ls $(FILTDIR)/*.[ch]; \
		ls $(FILTDIR)/Makefile; \
		ls $(TERMINFODIR)/Makefile; \
		ls $(TERMINFODIR)/*.ti; \
		ls $(ETCDIR)/*; \
	) | cpio -cova >$${GRABOUT:-lp.grab.cpio}

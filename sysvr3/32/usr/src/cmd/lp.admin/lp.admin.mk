#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp.admin:lp.admin.mk	1.6"
#
# Makefile for the System Administration menu for the
# LP Print Service package (sysadm packagemgmt/lpmgmt)
#


INS	=	install


SYSOWNER=	root
SYSGROUP=	sys

OWNER	=	lp
GROUP	=	sys

DMODES	=	0755
FMODES	=	0444
EMODES	=	0555


USRADMIN=	$(ROOT)/usr/admin

SYSDIRS	= \
		$(USRADMIN)/menu \
		$(USRADMIN)/menu/packagemgmt

MGMT	=	menu/packagemgmt/lpmgmt

DIRS	= \
		$(USRADMIN)/$(MGMT) \
		$(USRADMIN)/$(MGMT)/filters \
		$(USRADMIN)/$(MGMT)/forms \
		$(USRADMIN)/$(MGMT)/printers \
		$(USRADMIN)/$(MGMT)/service \
		$(USRADMIN)/$(MGMT)/status \
		$(USRADMIN)/$(MGMT)/users

FILES	= \
		$(MGMT)/DESC \
		$(MGMT)/filters/DESC \
		$(MGMT)/forms/DESC \
		$(MGMT)/printers/DESC \
		$(MGMT)/service/DESC \
		$(MGMT)/status/DESC \
		$(MGMT)/users/DESC \
		$(MGMT)/_S2.18 \
		$(MGMT)/_S2.25 \
		$(MGMT)/_S2.38 \
		$(MGMT)/_S2.43

EXECS	= \
		$(MGMT)/filters/add_f \
		$(MGMT)/filters/change_f \
		$(MGMT)/filters/delete_f \
		$(MGMT)/filters/list_f \
		$(MGMT)/forms/add_f \
		$(MGMT)/forms/change_f \
		$(MGMT)/forms/delete_f \
		$(MGMT)/forms/list_f \
		$(MGMT)/forms/mount_f \
		$(MGMT)/forms/status_f \
		$(MGMT)/forms/unmount_f \
		$(MGMT)/printers/accept_p \
		$(MGMT)/printers/add_p \
		$(MGMT)/printers/change_p \
		$(MGMT)/printers/delete_p \
		$(MGMT)/printers/disable_p \
		$(MGMT)/printers/enable_p \
		$(MGMT)/printers/list_p \
		$(MGMT)/printers/mount_p \
		$(MGMT)/printers/reject_p \
		$(MGMT)/printers/status_p \
		$(MGMT)/printers/unmount_p \
		$(MGMT)/service/default \
		$(MGMT)/service/start \
		$(MGMT)/service/stop \
		$(MGMT)/status/fonts \
		$(MGMT)/status/forms \
		$(MGMT)/status/printers \
		$(MGMT)/status/requests \
		$(MGMT)/status/service \
		$(MGMT)/users/list_u \
		$(MGMT)/users/priority_u


all:

install:	all
	for d in $(SYSDIRS); do if [ ! -d $$d ]; then mkdir $$d; fi; done
	$(CH)chown $(SYSOWNER) $(SYSDIRS)
	$(CH)chgrp $(SYSGROUP) $(SYSDIRS)
	$(CH)chmod $(DMODES) $(SYSDIRS)
	for d in $(DIRS); do if [ ! -d $$d ]; then mkdir $$d; fi; done
	$(CH)chown $(OWNER) $(DIRS)
	$(CH)chgrp $(GROUP) $(DIRS)
	$(CH)chmod $(DMODES) $(DIRS)
	ls $(FILES) | cpio -dp ${USRADMIN}
	cd ${USRADMIN}; $(CH)chown $(OWNER) $(FILES)
	cd ${USRADMIN}; $(CH)chgrp $(GROUP) $(FILES)
	cd ${USRADMIN}; $(CH)chmod $(EMODES) $(FILES)
	ls $(EXECS) | cpio -dp ${USRADMIN}
	cd ${USRADMIN}; $(CH)chown $(OWNER) $(EXECS)
	cd ${USRADMIN}; $(CH)chgrp $(GROUP) $(EXECS)
	cd ${USRADMIN}; $(CH)chmod $(EMODES) $(EXECS)

clean:

clobber:	clean

strip:

gen:
	gen

continue:
	gen continue

continue2:
	gen continue2

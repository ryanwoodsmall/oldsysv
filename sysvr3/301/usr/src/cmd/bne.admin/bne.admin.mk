#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bne.admin:bne.admin.mk	2.3"
#
#	This makefile makes appropriate directories
#	and copies the simple admin shells into
#	the uucpmgmt directory
#

OWNER=uucp
GRP=daemon
MGMT=$(ROOT)/usr/admin/menu/packagemgmt/uucpmgmt


install:
	-mkdir $(ROOT)/usr
	-mkdir $(ROOT)/usr/admin
	-mkdir $(ROOT)/usr/admin/menu
	-mkdir $(ROOT)/usr/admin/menu/packagemgmt
	-mkdir $(MGMT)
	cp DESC $(MGMT)/DESC
	cp adddevice $(MGMT)/adddevice
	cp addpoll $(MGMT)/addpoll
	cp addport $(MGMT)/addport
	cp addsystem $(MGMT)/addsystem
	cp deldevice $(MGMT)/deldevice
	cp delpoll $(MGMT)/delpoll
	cp delport $(MGMT)/delport
	cp delsystem $(MGMT)/delsystem
	cp devicemgmt $(MGMT)/devicemgmt
	cp lsdevice $(MGMT)/lsdevice
	cp lspoll $(MGMT)/lspoll
	cp lsport $(MGMT)/lsport
	cp lssystem $(MGMT)/lssystem
	cp modifyport $(MGMT)/modifyport
	cp pollmgmt $(MGMT)/pollmgmt
	cp portmgmt $(MGMT)/portmgmt
	cp systemmgmt $(MGMT)/systemmgmt
	cp trysystem $(MGMT)/trysystem
	cp _delinittab $(MGMT)/_delinittab
	cp _initprint $(MGMT)/_initprint
	cp _systemprint $(MGMT)/_systemprint
	cp _deviceprint $(MGMT)/_deviceprint
	$(CH)-chgrp $(GRP) $(MGMT)/*
	$(CH)-chmod 555 $(MGMT)/[a-Z]*
	$(CH)-chown $(OWNER) $(MGMT)/*
	$(CH)-chmod 755 $(MGMT)
	$(CH)-chgrp $(GRP) $(MGMT)
	$(CH)-chown $(OWNER) $(MGMT)
	$(CH)-chmod 755 $(ROOT)/usr/admin/menu/packagemgmt
	$(CH)-chgrp bin $(ROOT)/usr/admin/menu/packagemgmt
	$(CH)-chown root $(ROOT)/usr/admin/menu/packagemgmt
	$(CH)-chmod 755 $(ROOT)/usr/admin/menu
	$(CH)-chgrp bin $(ROOT)/usr/admin/menu
	$(CH)-chown root $(ROOT)/usr/admin/menu
	$(CH)-chmod 755 $(ROOT)/usr/admin
	$(CH)-chgrp bin $(ROOT)/usr/admin
	$(CH)-chown root $(ROOT)/usr/admin
	$(CH)-chmod 775 $(ROOT)/usr
	$(CH)-chgrp sys $(ROOT)/usr
	$(CH)-chown root $(ROOT)/usr

clobber:

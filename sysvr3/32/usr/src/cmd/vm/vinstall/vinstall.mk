#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T Technologies, Inc.
#	All Rights Reserved
#
#ident	"@(#)vm.vinstall:src/vinstall/vinstall.mk	1.16"
#
# vinstall.mk -- the makefile for the install subsystem of FACE
#

INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
USR =	$(ROOT)/usr
CFLAGS=	-O
LDFLAGS= -s

ACMDS = usrmgmt viewsetup servmgmt addserv delserv modserv

LCMDS = vmsetup   vsetup

ADMINS = DESC

VBIN = $(USR)/vmsys/bin

BIN = $(USR)/vmsys $(USR)/vmsys/bin $(USR)/oasys $(USR)/oasys/bin

MKDIR = mkdir

SYSADM = $(USR)/admin/menu/packagemgmt/FACE

TMP = $(USR)/oasys/tmp

TERRLOG = $(TMP)/TERRLOG

all:		compile

compile:  	$(ACMDS) $(LCMDS)

install: 	compile  $(BIN) $(SYSADM) $(TMP) $(TERRLOG)
		@set +e;\
		for i in $(ACMDS);\
		do\
			install -f $(SYSADM) $$i;\
			$(CH)chmod 444 $(SYSADM)/$$i;\
			$(CH)chgrp sys $(SYSADM)/$$i;\
			$(CH)chown root $(SYSADM)/$$i;\
		done;\
		for i in $(ADMINS);\
		do\
			install -f $(SYSADM) $$i;\
			$(CH)chmod 444 $(SYSADM)/$$i;\
			$(CH)chgrp sys $(SYSADM)/$$i;\
		done;\
		for i in $(LCMDS);\
		do\
			install -f $(VBIN) $$i;\
		done

clean:
	@set +e;\
	for i in $(ACMDS);\
	do\
		/bin/rm -f $$i;\
	done;\
	for i in $(LCMDS);\
	do\
		/bin/rm -f $$i;\
	done

clobber:	clean

$(BIN):
	$(MKDIR) $@ ;\
	$(CH)chmod 755 $@

$(SYSADM):
	$(MKDIR) $@ 
	
$(TMP):
	$(MKDIR) $@ ;\
	$(CH)chmod 777 $@

$(TERRLOG):
	> $@ ;\
	$(CH)chmod 622 $@

.c:
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

.o:
	$(CC) $(LDFLAGS) -o $@ $<
.sh :
	rm -f $@
	/bin/cp $< $@
	$(CH)chmod 755 $@


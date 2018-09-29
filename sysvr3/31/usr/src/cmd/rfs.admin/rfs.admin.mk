#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfs.admin:rfs.admin.mk	1.6"
ROOT =
RFSADMIN = $(ROOT)/usr/admin/menu/packagemgmt/rfsmgmt
TESTDIR = .
INSDIR = $(ROOT)/usr/lbin
INC = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
LDFLAGS = -lns $(LDLIBS)
FRC =

all: rfsrunning

rfsrunning: rfsrunning.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/rfsrunning rfsrunning.c $(LDFLAGS)

install: all
	install -f $(INSDIR) $(TESTDIR)/rfsrunning
	if [ ! -d $(RFSADMIN) ] ; then mkdir $(RFSADMIN) ; fi
	cp ./DESC $(RFSADMIN)
	if [ ! -d $(RFSADMIN)/resrcmgmt ] ; then mkdir $(RFSADMIN)/resrcmgmt ; fi
	cp ./resrcmgmt/DESC $(RFSADMIN)/resrcmgmt
	if [ ! -d $(RFSADMIN)/resrcmgmt/advmgmt ] ; then mkdir $(RFSADMIN)/resrcmgmt/advmgmt ; fi
	cp ./resrcmgmt/advmgmt/DESC $(RFSADMIN)/resrcmgmt/advmgmt
	cp ./resrcmgmt/advmgmt/lsadv $(RFSADMIN)/resrcmgmt/advmgmt
	cp ./resrcmgmt/advmgmt/advauto $(RFSADMIN)/resrcmgmt/advmgmt
	cp ./resrcmgmt/advmgmt/advnow $(RFSADMIN)/resrcmgmt/advmgmt
	cp ./resrcmgmt/advmgmt/lsinuse $(RFSADMIN)/resrcmgmt/advmgmt
	cp ./resrcmgmt/advmgmt/unadvnow $(RFSADMIN)/resrcmgmt/advmgmt
	cp ./resrcmgmt/advmgmt/unadvauto $(RFSADMIN)/resrcmgmt/advmgmt
	if [ ! -d $(RFSADMIN)/resrcmgmt/mountmgmt ] ; then mkdir $(RFSADMIN)/resrcmgmt/mountmgmt ; fi
	cp ./resrcmgmt/mountmgmt/DESC $(RFSADMIN)/resrcmgmt/mountmgmt
	cp ./resrcmgmt/mountmgmt/mntauto $(RFSADMIN)/resrcmgmt/mountmgmt
	cp ./resrcmgmt/mountmgmt/unmntauto $(RFSADMIN)/resrcmgmt/mountmgmt
	cp ./resrcmgmt/mountmgmt/mntnow $(RFSADMIN)/resrcmgmt/mountmgmt
	cp ./resrcmgmt/mountmgmt/unmntnow $(RFSADMIN)/resrcmgmt/mountmgmt
	cp ./resrcmgmt/mountmgmt/lsavail $(RFSADMIN)/resrcmgmt/mountmgmt
	cp ./resrcmgmt/mountmgmt/lsmount $(RFSADMIN)/resrcmgmt/mountmgmt
	if [ ! -d $(RFSADMIN)/startstop ] ; then mkdir $(RFSADMIN)/startstop ; fi
	cp ./startstop/DESC $(RFSADMIN)/startstop
	cp ./startstop/startrfs $(RFSADMIN)/startstop
	cp ./startstop/stoprfs $(RFSADMIN)/startstop
	cp ./startstop/setauto $(RFSADMIN)/startstop
	cp ./startstop/isrfson $(RFSADMIN)/startstop
	cp ./setuprfs $(RFSADMIN)
	if [ ! -d $(RFSADMIN)/confgmgmt ] ; then mkdir $(RFSADMIN)/confgmgmt ; fi
	cp ./confgmgmt/DESC $(RFSADMIN)/confgmgmt
	cp ./confgmgmt/idmapping $(RFSADMIN)/confgmgmt
	cp ./confgmgmt/lsmember $(RFSADMIN)/confgmgmt
	cp ./confgmgmt/delmember $(RFSADMIN)/confgmgmt
	cp ./confgmgmt/addmember $(RFSADMIN)/confgmgmt
	cp ./confgmgmt/showconfg $(RFSADMIN)/confgmgmt
	$(CH)cd $(RFSADMIN); find . -type f -print | xargs chmod 755
	$(CH)cd $(RFSADMIN); find . -print | xargs chown root
	$(CH)cd $(RFSADMIN); find . -print | xargs chgrp sys

clean:
	rm -f rfsrunning.o

clobber: clean
	rm -f $(TESTDIR)/rfsrunning
FRC:

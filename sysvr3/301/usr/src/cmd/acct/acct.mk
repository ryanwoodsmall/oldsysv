#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)acct:acct.mk	1.9.1.11"
ROOT =
TESTDIR = .
FRC =
INS = install
ARGS =
INSDIR = $(ROOT)/usr/lib/acct
ADMDIR = $(ROOT)/usr/adm
ROOTBIN = $(ROOT)/bin
ETCINIT = $(ROOT)/etc/init.d
WKDIR	= $(ADMDIR) $(ADMDIR)/acct $(ADMDIR)/acct/nite $(ADMDIR)/acct/fiscal $(ADMDIR)/acct/sum
CC = cc
CFLAGS = -O
LFLAGS = -s
LIB = lib/a.a

all:	library acctcms acctcom acctcon1\
	acctcon2 acctdisk acctdusg acctmerg accton\
	acctprc1 acctprc2 acctwtmp\
	diskusg fwtmp wtmpfix\
	chargefee ckpacct dodisk lastlogin\
	monacct nulladm prctmp prdaily\
	prtacct remove runacct acct\
	shutacct startup turnacct holtable \
	awkecms awkelus

library:
		cd lib; make "INS=$(INS)" "CC=$(CC)" "CFLAGS=$(CFLAGS)"

acctcms:	$(LIB) acctcms.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctcms.c $(LIB) -o $(TESTDIR)/acctcms

acctcom:	$(LIB) acctcom.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctcom.c $(LIB) -o $(TESTDIR)/acctcom

acctcon1:	$(LIB) acctcon1.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctcon1.c $(LIB) -o $(TESTDIR)/acctcon1

acctcon2:	acctcon2.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctcon2.c -o $(TESTDIR)/acctcon2

acctdisk:	acctdisk.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctdisk.c -o $(TESTDIR)/acctdisk

acctdusg:	acctdusg.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctdusg.c -o $(TESTDIR)/acctdusg

acctmerg:	$(LIB) acctmerg.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctmerg.c $(LIB) -o $(TESTDIR)/acctmerg

accton:		accton.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			accton.c -o $(TESTDIR)/accton

acctprc1:	$(LIB) acctprc1.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctprc1.c $(LIB) -o $(TESTDIR)/acctprc1

acctprc2:	acctprc2.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctprc2.c -o $(TESTDIR)/acctprc2

acctwtmp:	acctwtmp.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctwtmp.c -o $(TESTDIR)/acctwtmp

diskusg:	diskusg.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			diskusg.c -o $(TESTDIR)/diskusg

fwtmp:		fwtmp.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			fwtmp.c -o $(TESTDIR)/fwtmp

wtmpfix:	wtmpfix.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			wtmpfix.c -o $(TESTDIR)/wtmpfix

chargefee:	chargefee.sh $(FRC)
		cp chargefee.sh $(TESTDIR)/chargefee

ckpacct:	ckpacct.sh $(FRC)
		cp ckpacct.sh $(TESTDIR)/ckpacct

dodisk:		dodisk.sh $(FRC)
		cp dodisk.sh $(TESTDIR)/dodisk

monacct:	monacct.sh $(FRC)
		cp monacct.sh $(TESTDIR)/monacct

lastlogin:	lastlogin.sh $(FRC)
		cp lastlogin.sh $(TESTDIR)/lastlogin

nulladm:	nulladm.sh $(FRC)
		cp nulladm.sh $(TESTDIR)/nulladm

prctmp:		prctmp.sh $(FRC)
		cp prctmp.sh $(TESTDIR)/prctmp

prdaily:	prdaily.sh $(FRC)
		cp prdaily.sh $(TESTDIR)/prdaily

prtacct:	prtacct.sh $(FRC)
		cp prtacct.sh $(TESTDIR)/prtacct

remove:		remove.sh $(FRC)
		cp remove.sh $(TESTDIR)/remove

runacct:	runacct.sh $(FRC)
		cp runacct.sh $(TESTDIR)/runacct

acct:		acct.sh $(FRC)
		cp acct.sh $(TESTDIR)/acct

shutacct:	shutacct.sh $(FRC)
		cp shutacct.sh $(TESTDIR)/shutacct

startup:	startup.sh $(FRC)
		cp startup.sh $(TESTDIR)/startup

turnacct:	turnacct.sh $(FRC)
		cp turnacct.sh $(TESTDIR)/turnacct

holtable:	holidays $(FRC)

awkecms:	ptecms.awk $(FRC)

awkelus:	ptelus.awk $(FRC)

$(INSDIR):
		mkdir $@;
		chmod 775 $@;
		$(CH) chown bin $@
		$(CH) chgrp bin $@

$(WKDIR):
		mkdir $@;
		chmod 775 $@;
		$(CH) chown adm $@
		$(CH) chgrp adm $@

install:	all $(INSDIR) $(ADMDIR)
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcms
		$(INS) -f $(ROOTBIN) $(TESTDIR)/acctcom
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcon1
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcon2
		$(INS) -f $(INSDIR) $(TESTDIR)/acctdisk
		$(INS) -f $(INSDIR) $(TESTDIR)/acctdusg
		$(INS) -f $(INSDIR) $(TESTDIR)/acctmerg
		$(INS) -f $(INSDIR) -u root -g adm -m 4755 $(TESTDIR)/accton
		$(INS) -f $(INSDIR) $(TESTDIR)/acctprc1
		$(INS) -f $(INSDIR) $(TESTDIR)/acctprc2
		$(INS) -f $(INSDIR) $(TESTDIR)/acctwtmp
		$(INS) -f $(INSDIR) $(TESTDIR)/fwtmp
		$(INS) -f $(INSDIR) $(TESTDIR)/diskusg
		$(INS) -f $(INSDIR) $(TESTDIR)/wtmpfix
		$(INS) -f $(INSDIR) $(TESTDIR)/chargefee
		$(INS) -f $(INSDIR) $(TESTDIR)/ckpacct
		$(INS) -f $(INSDIR) $(TESTDIR)/dodisk
		$(INS) -f $(INSDIR) $(TESTDIR)/monacct
		$(INS) -f $(INSDIR) $(TESTDIR)/lastlogin
		$(INS) -f $(INSDIR) $(TESTDIR)/nulladm
		$(INS) -f $(INSDIR) $(TESTDIR)/prctmp
		$(INS) -f $(INSDIR) $(TESTDIR)/prdaily
		$(INS) -f $(INSDIR) $(TESTDIR)/prtacct
		$(INS) -f $(INSDIR) $(TESTDIR)/remove
		$(INS) -f $(INSDIR) $(TESTDIR)/runacct
		$(INS) -f $(ETCINIT) -u root -g sys -m 444 $(TESTDIR)/acct
		$(INS) -f $(INSDIR) $(TESTDIR)/shutacct
		$(INS) -f $(INSDIR) $(TESTDIR)/startup
		$(INS) -f $(INSDIR) $(TESTDIR)/turnacct
		$(INS) -f $(INSDIR) $(TESTDIR)/holidays
		$(INS) -f $(INSDIR) $(TESTDIR)/ptecms.awk
		$(INS) -f $(INSDIR) $(TESTDIR)/ptelus.awk
	rm -rf $(ROOT)/etc/rc2.d/S22acct
	rm -rf $(ROOT)/etc/rc0.d/K22acct
	/bin/ln $(ETCINIT)/acct $(ROOT)/etc/rc2.d/S22acct
	/bin/ln $(ETCINIT)/acct $(ROOT)/etc/rc0.d/K22acct

clean:
		-rm -f *.o
		cd lib; make clean

clobber:	clean
		-rm -f					\
			$(TESTDIR)/acctcms		\
			$(TESTDIR)/acctcom		\
			$(TESTDIR)/acctcon1		\
			$(TESTDIR)/acctcon2		\
			$(TESTDIR)/acctdisk		\
			$(TESTDIR)/diskusg		\
			$(TESTDIR)/acctdusg		\
			$(TESTDIR)/acctmerg		\
			$(TESTDIR)/accton		\
			$(TESTDIR)/acctprc1		\
			$(TESTDIR)/acctprc2		\
			$(TESTDIR)/acctwtmp		\
			$(TESTDIR)/fwtmp		\
			$(TESTDIR)/wtmpfix
		-rm -f					\
			$(TESTDIR)/chargefee		\
			$(TESTDIR)/ckpacct		\
			$(TESTDIR)/dodisk		\
			$(TESTDIR)/lastlogin		\
			$(TESTDIR)/nulladm		\
			$(TESTDIR)/monacct		\
			$(TESTDIR)/prctmp		\
			$(TESTDIR)/prdaily		\
			$(TESTDIR)/prtacct		\
			$(TESTDIR)/remove		\
			$(TESTDIR)/runacct		\
			$(TESTDIR)/acct			\
			$(TESTDIR)/shutacct		\
			$(TESTDIR)/startup		\
			$(TESTDIR)/turnacct
		cd lib; make clobber

FRC:

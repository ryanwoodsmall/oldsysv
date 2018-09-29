#	@(#)acct.mk	1.9 of 9/23/83 #
ROOT =
TESTDIR = .
FRC =
INS = install
ARGS =
INSDIR = $(ROOT)/usr/lib/acct
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
	prtacct remove runacct\
	shutacct startup turnacct holtable \
	awkecms awkelus

library:
		cd lib; make "INS=$(INS)" "CC=$(CC)" "CFLAGS=$(CFLAGS)"

acctcms:	library acctcms.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctcms.c $(LIB) -o $(TESTDIR)/acctcms
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcms

acctcom:	library acctcom.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctcom.c $(LIB) -o $(TESTDIR)/acctcom
		$(INS) $(TESTDIR)/acctcom $(ROOT)/bin

acctcon1:	library acctcon1.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctcon1.c $(LIB) -o $(TESTDIR)/acctcon1
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcon1

acctcon2:	acctcon2.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctcon2.c -o $(TESTDIR)/acctcon2
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcon2

acctdisk:	acctdisk.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctdisk.c -o $(TESTDIR)/acctdisk
		$(INS) -f $(INSDIR) $(TESTDIR)/acctdisk

acctdusg:	acctdusg.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctdusg.c -o $(TESTDIR)/acctdusg
		$(INS) -f $(INSDIR) $(TESTDIR)/acctdusg

acctmerg:	library acctmerg.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctmerg.c $(LIB) -o $(TESTDIR)/acctmerg
		$(INS) -f $(INSDIR) $(TESTDIR)/acctmerg

accton:		accton.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			accton.c -o $(TESTDIR)/accton
		$(INS) -f $(INSDIR) -u root -g adm -m 4755 $(TESTDIR)/accton

acctprc1:	library acctprc1.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctprc1.c $(LIB) -o $(TESTDIR)/acctprc1
		$(INS) -f $(INSDIR) $(TESTDIR)/acctprc1

acctprc2:	acctprc2.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctprc2.c -o $(TESTDIR)/acctprc2
		$(INS) -f $(INSDIR) $(TESTDIR)/acctprc2

acctwtmp:	acctwtmp.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			acctwtmp.c -o $(TESTDIR)/acctwtmp
		$(INS) -f $(INSDIR) $(TESTDIR)/acctwtmp

diskusg:	diskusg.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			diskusg.c -o $(TESTDIR)/diskusg
		$(INS) -f $(INSDIR) $(TESTDIR)/diskusg

fwtmp:		fwtmp.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			fwtmp.c -o $(TESTDIR)/fwtmp
		$(INS) -f $(INSDIR) $(TESTDIR)/fwtmp

wtmpfix:	wtmpfix.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG) \
			wtmpfix.c -o $(TESTDIR)/wtmpfix
		$(INS) -f $(INSDIR) $(TESTDIR)/wtmpfix

chargefee:	chargefee.sh $(FRC)
		cp chargefee.sh $(TESTDIR)/chargefee
		$(INS) -f $(INSDIR) $(TESTDIR)/chargefee

ckpacct:	ckpacct.sh $(FRC)
		cp ckpacct.sh $(TESTDIR)/ckpacct
		$(INS) -f $(INSDIR) $(TESTDIR)/ckpacct

dodisk:		dodisk.sh $(FRC)
		cp dodisk.sh $(TESTDIR)/dodisk
		$(INS) -f $(INSDIR) $(TESTDIR)/dodisk

monacct:	monacct.sh $(FRC)
		cp monacct.sh $(TESTDIR)/monacct
		$(INS) -f $(INSDIR) $(TESTDIR)/monacct

lastlogin:	lastlogin.sh $(FRC)
		cp lastlogin.sh $(TESTDIR)/lastlogin
		$(INS) -f $(INSDIR) $(TESTDIR)/lastlogin

nulladm:	nulladm.sh $(FRC)
		cp nulladm.sh $(TESTDIR)/nulladm
		$(INS) -f $(INSDIR) $(TESTDIR)/nulladm

prctmp:		prctmp.sh $(FRC)
		cp prctmp.sh $(TESTDIR)/prctmp
		$(INS) -f $(INSDIR) $(TESTDIR)/prctmp

prdaily:	prdaily.sh $(FRC)
		cp prdaily.sh $(TESTDIR)/prdaily
		$(INS) -f $(INSDIR) $(TESTDIR)/prdaily

prtacct:	prtacct.sh $(FRC)
		cp prtacct.sh $(TESTDIR)/prtacct
		$(INS) -f $(INSDIR) $(TESTDIR)/prtacct

remove:		remove.sh $(FRC)
		cp remove.sh $(TESTDIR)/remove
		$(INS) -f $(INSDIR) $(TESTDIR)/remove

runacct:	runacct.sh $(FRC)
		cp runacct.sh $(TESTDIR)/runacct
		$(INS) -f $(INSDIR) $(TESTDIR)/runacct

shutacct:	shutacct.sh $(FRC)
		cp shutacct.sh $(TESTDIR)/shutacct
		$(INS) -f $(INSDIR) $(TESTDIR)/shutacct

startup:	startup.sh $(FRC)
		cp startup.sh $(TESTDIR)/startup
		$(INS) -f $(INSDIR) $(TESTDIR)/startup

turnacct:	turnacct.sh $(FRC)
		cp turnacct.sh $(TESTDIR)/turnacct
		$(INS) -f $(INSDIR) $(TESTDIR)/turnacct

holtable:	holidays $(FRC)
		$(INS) -f $(INSDIR) $(TESTDIR)/holidays

awkecms:	ptecms.awk $(FRC)
		$(INS) -f $(INSDIR) $(TESTDIR)/ptecms.awk

awkelus:	ptelus.awk $(FRC)
		$(INS) -f $(INSDIR) $(TESTDIR)/ptelus.awk

install:
		make -f acct.mk $(ARGS) FFLAG=$(FFLAG)

clean:
		-rm -f *.o
		cd lib; make clean

clobber:	clean
		-rm -f acctcms acctcom acctcon1 acctcon2 acctdisk diskusg\
			acctdusg acctmerg accton acctprc1 acctprc2 acctwtmp\
			fwtmp wtmpfix
		-rm -f chargefee ckpacct dodisk lastlogin nulladm\
			monacct prctmp prdaily prtacct remove runacct\
			shutacct startup turnacct
		cd lib; make clobber

FRC:

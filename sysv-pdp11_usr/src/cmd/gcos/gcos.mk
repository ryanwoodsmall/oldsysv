#	@(#)gcos.mk	1.2
GERTS = kmcgerts
CONFIG = dumconfig
CFLAGS = -O
LDFLAGS = -s -n
LIBES =
INS = :
FRC =
TESTDIR = .
COMDIR = /usr/bin
DEMDIR = /usr/lib

all:
	echo cat README

spoolers:	dpr fget fsend gcat gcosmail
daemons:	dpd fget.demon

dpr::	dpr.c spool.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/dpr dpr.c $(LIBES)
dpr::
	$(INS) -n $(COMDIR) $(TESTDIR)/dpr

fget::	fget.o $(CONFIG).o $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/fget fget.o $(CONFIG).o $(LIBES)
fget::
	$(INS) -n $(COMDIR) $(TESTDIR)/fget
fget.o:	fget.c spool.c
	$(CC) $(CFLAGS) -c fget.c

fsend::	fsend.c spool.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/fsend fsend.c $(LIBES)
fsend::
	$(INS) -n $(COMDIR) $(TESTDIR)/fsend

gcat::	gcat.c spool.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/gcat gcat.c $(LIBES)
gcat::
	$(INS) -n $(COMDIR) $(TESTDIR)/gcat

gcosmail::	gcosmail.c spool.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/gcosmail gcosmail.c $(LIBES)
gcosmail::
	$(INS) -n $(COMDIR) $(TESTDIR)/gcosmail

ibm::	ibm.c spool.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/ibm ibm.c $(LIBES)
ibm::
	$(INS) -n $(COMDIR) $(TESTDIR)/ibm

labmake::	labmake.c spool.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/labmake labmake.c $(LIBES)
labmake::
	$(INS) -n $(COMDIR) $(TESTDIR)/labmake

tekstare::	tekstare.c spool.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/tekstare tekstare.c $(LIBES)
tekstare::
	$(INS) -n $(COMDIR) $(TESTDIR)/tekstare

dpd::	phone.dpd.o $(GERTS).o $(CONFIG).o gcset.o $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/dpd phone.dpd.o $(GERTS).o\
		$(CONFIG).o gcset.o $(LIBES)
dpd::
	$(INS) -n $(DEMDIR) $(TESTDIR)/dpd
phone.dpd.o:	phone.dpd.c daemon.c daemon0.c $(FRC)

fget.demon::	phone.fgd.o $(GERTS).o $(CONFIG).o cgset.o $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/fget.demon phone.fgd.o $(GERTS).o\
		$(CONFIG).o cgset.o $(LIBES)
fget.demon::
	$(INS) -n $(DEMDIR) $(TESTDIR)/fget.demon
phone.fgd.o:	phone.fgd.c fgdemon.c daemon0.c $(FRC)

$(GERTS).o:	$(GERTS).c $(FRC)
$(CONFIG).o:	$(CONFIG).c gcos.h $(FRC)
cgset.o:	cgset.c
gcset.o:	gcset.c

install:
	make -f gcos.mk all INS=/etc/install FRC=$(FRC)\
		COMDIR=$(COMDIR) DEMDIR=$(DEMDIR) TESTDIR=$(TESTDIR)\
		"CFLAGS=$(CFLAGS)" "LDFLAGS=$(LDFLAGS)" "LIBES=$(LIBES)"\
		CONFIG=$(CONFIG) GERTS=$(GERTS)

clean:
	rm -f *.o

clobber:	clean
	cd $(TESTDIR);\
	rm -f dpr fget fsend gcat gcosmail ibm labmake tekstare\
		dpd fget.demon

FRC:

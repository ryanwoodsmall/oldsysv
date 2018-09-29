#	@(#)net.mk	1.1
CFLAGS = -O
LDFLAGS = -s
INCRT = /usr/include
INS = :
FRC =
TESTDIR = .
COMDIR = /usr/bin
DEMDIR = /usr/bin

all:	net pcldaemon
net::\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/pcl.h\
	$(INCRT)/sys/utsname.h\
	$(INCRT)/signal.h\
	$(INCRT)/errno.h\
	$(INCRT)/pwd.h\
	net.h\
	net.c\
	$(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/net net.c

net::
	$(INS) -n $(COMDIR) $(TESTDIR)/net

pcldaemon::\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/pcl.h\
	$(INCRT)/signal.h\
	$(INCRT)/stdio.h\
	$(INCRT)/setjmp.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/pwd.h\
	net.h\
	pcldaemon.c\
	$(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/pcldaemon pcldaemon.c

pcldaemon::
	$(INS) -n $(COMDIR) $(TESTDIR)/pcldaemon

install:
	make -f net.mk all INS=/etc/install FRC=$(FRC)\
		COMDIR=$(COMDIR) DEMDIR=$(DEMDIR) TESTDIR=$(TESTDIR)\
		"CFLAGS=$(CFLAGS)" "LDFLAGS=$(LDFLAGS)" "INCRT=$(INCRT)"

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/net $(TESTDIR)/pcldaemon

FRC:

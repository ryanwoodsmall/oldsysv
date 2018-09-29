# @(#)dzb.mk	6.1
TESTDIR = .
FRC =
INS = /etc/install
CFLAGS = -O -I$(INCRT) 

CPP = /lib/cpp
KASB = kasb
CC = cc

SFILES =\
	input.s \
	main.s \
	output.s \
	subs.s

all:	dzkmcb.o dzbload dzbset

dzkmcb.o: $(SFILES) $(FRC)
	$(CPP) main.s | $(KASB) -o $(TESTDIR)/dzkmcb.o
	-chmod 664 $(TESTDIR)/dzkmcb.o
	-chown bin $(TESTDIR)/dzkmcb.o
	-chgrp bin $(TESTDIR)/dzkmcb.o
	$(INS) -n /lib dzkmcb.o

dzbload: dzbload.sh $(FRC)
	cp dzbload.sh $(TESTDIR)/dzbload
	-chmod 664 $(TESTDIR)/dzbload
	-chown bin $(TESTDIR)/dzbload
	-chgrp bin $(TESTDIR)/dzbload
	$(INS) -n /etc dzbload

dzbset: dzbset.c $(FRC)
	$(CC) $(CFLAGS) -o dzbset dzbset.c
	-chown bin $(TESTDIR)/dzbset
	-chgrp bin $(TESTDIR)/dzbset
	-chmod 664 $(TESTDIR)/dzbset
	$(INS) -n /etc dzbset
clean:

clobber:	clean
	-rm -f dzkmcb.o
	-rm -f dzbload
	-rm -f dzbset

FRC:

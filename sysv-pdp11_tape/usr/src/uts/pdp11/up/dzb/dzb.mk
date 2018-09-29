# @(#)dzb.mk	1.1
TESTDIR = .
FRC =
INS = /etc/install

CPP = /lib/cpp
KASB = kasb

SFILES =\
	input.s \
	main.s \
	output.s \
	subs.s

all:	dzkmcb.o dzbload

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

clean:

clobber:	clean
	-rm -f dzkmcb.o
	-rm -f dzbload

FRC:

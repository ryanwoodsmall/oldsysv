#	@(#)makefile	1.1
#
#	Makefile for file command -- necessary to install /etc/magic
#
TESTDIR=.
CFLAGS=	-O
INS=	/etc/install
INSDIR=	/usr/lbin
MINSDIR=/etc

all:	file

file:	file.c
	$(CC) $(CFLAGS) $(LDFLAGS) file.c
	mv a.out $(TESTDIR)/file

install: all
	$(INS) -n $(INSDIR) $(TESTDIR)/file
	$(INS) -n $(MINSDIR) magic

clean:
	-rm -f file.o

clobber: clean
	-rm -f $(TESTDIR)/file

#	@(#)file.mk	1.3
#	@(#)makefile	1.1
#
#	Makefile for file command -- necessary to install /etc/magic
#
ROOT =
TESTDIR=.
INS = install
INSDIR = $(ROOT)/bin
MINSDIR = $(ROOT)/etc

all:	file

file:	file.c
	$(CC) $(CFLAGS) $(LDFLAGS) file.c -o $(TESTDIR)/file

install: all
	$(INS) -n $(INSDIR) $(TESTDIR)/file
	$(INS) -n $(MINSDIR) magic

clean:
	-rm -f file.o

clobber: clean
	-rm -f $(TESTDIR)/file

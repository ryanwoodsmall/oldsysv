#	@(#)libg.mk	1.1
INS=/etc/install
TESTDIR=.
all:	libg.a

libg.a: dbxxx.s
	as -o $(TESTDIR)/libg.a dbxxx.s

install: all
	$(INS) -n /usr/lib $(TESTDIR)/libg.a

clean:
	rm -f *.o
clobber: clean
	rm -f libg.a

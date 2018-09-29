#	@(#)ls.mk	1.4
ROOT =
TESTDIR = .
INS = /etc/install
LDFLAGS = -O -s -i
LIBCURSES = $(ROOT)/usr/lib/libcurses.a

all: ls

ls: ls.c
	$(CC) -DSINGLE $(LDFLAGS) -o $(TESTDIR)/ls ls.c $(LIBCURSES)
#	$(CC) $(LDFLAGS) -DNOTERMINFO -o $(TESTDIR)/ls ls.c

install: all
	$(INS) -n /bin $(TESTDIR)/ls

clean:
	-rm -f ls.o

clobber: clean
	-rm -f $(TESTDIR)/ls

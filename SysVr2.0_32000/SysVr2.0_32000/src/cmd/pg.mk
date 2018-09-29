#	@(#)pg.mk	1.3
ROOT =
TESTDIR = .
INS = /etc/install
LIBCURSES=$(ROOT)/usr/lib/libcurses.a
LDFLAGS = -O -s -i

all: pg

pg: pg.c
	$(CC) -DSINGLE $(LDFLAGS) -o $(TESTDIR)/pg pg.c $(LIBCURSES)

install: all
	$(INS) -n $(ROOT)/usr/bin $(TESTDIR)/pg

clean:
	-rm -f pg.o

clobber: clean
	-rm -f $(TESTDIR)/pg

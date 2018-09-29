#	@(#)tabs.mk	1.3
ROOT=
TESTDIR = .
FRC =
INS = /etc/install
INSDIR = $(ROOT)/usr/bin 
LDFLAGS = -O -s
LIBCURSES=$(ROOT)/usr/lib/libcurses.a

all: tabs

tabs: tabs.c $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/tabs tabs.c $(LIBCURSES)

install: all
	$(INS) -f $(INSDIR) $(TESTDIR)/tabs

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/tabs

FRC:

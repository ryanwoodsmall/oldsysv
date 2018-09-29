#	@(#)tput.mk	1.3
ROOT=
TESTDIR = .
FRC =
INS = :
INSDIR = $(ROOT)/usr/bin 
LIBCURSES=$(ROOT)/usr/lib/libcurses.a
LDFLAGS = -O -s

all:	tput

tput:	tput.c $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/tput tput.c $(LIBCURSES)
	$(INS) -f $(INSDIR) $(TESTDIR)/tput

install:
	$(MAKE) -f tput.mk INS="install -f $(INSDIR)"

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(TESTDIR)/tput

FRC:


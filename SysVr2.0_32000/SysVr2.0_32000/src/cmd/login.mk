#	%W%
ROOT =
TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/bin 
LDFLAGS = -O -s
CONS=-DCONSOLE='"/dev/console"'

all: login

login: login.c $(FRC)
	$(CC) $(CONS) $(LDFLAGS) -o $(TESTDIR)/login login.c  

test:
	rtest $(TESTDIR)/login

install: all
	$(INS) -o -n $(INSDIR) $(TESTDIR)/login

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/login

FRC:

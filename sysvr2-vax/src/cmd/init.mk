#	@(#)init.mk	1.2
TESTDIR = .
FRC =
INS = /etc/install
INSDIR = /etc 
LDFLAGS = -O -s

all: init

init: init.c $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/init init.c  

test:
	rtest $(TESTDIR)/init

install: all
	$(INS) -o $(TESTDIR)/init $(INSDIR)
	-rm -f /bin/telinit
	ln /etc/init /bin/telinit

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/init

FRC:

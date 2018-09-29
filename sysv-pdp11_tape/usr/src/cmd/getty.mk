#	@(#)getty.mk	1.1
TESTDIR = .
FRC =
INS = /etc/install
INSDIR = /etc 
LDFLAGS = -s -n

all: getty

getty: getty.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/getty getty.c  

test:
	rtest $(TESTDIR)/getty

install: all
	$(INS) -o $(TESTDIR)/getty $(INSDIR)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/getty

FRC:

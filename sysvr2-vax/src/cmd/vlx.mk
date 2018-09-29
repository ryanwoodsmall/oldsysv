#	@(#)vlx.mk	1.1
TESTDIR = .
FRC =
INS = /etc/install
INSDIR = /etc 
LDFLAGS = -O -s

all: stest vlx

vlx: vlx.c $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/vlx vlx.c  

stest:
	@set +e;if pdp11; then echo "\nNO VLX on PDP11\n";exit 1;fi

test:
	rtest $(TESTDIR)/vlx

install: all
	$(INS) $(TESTDIR)/vlx $(INSDIR)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/vlx

FRC:

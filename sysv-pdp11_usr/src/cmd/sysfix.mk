#	@(#)sysfix.mk	1.1
TESTDIR = .
FRC =
INS = /etc/install
INSDIR = /etc 
LDFLAGS = -O -s

all: stest sysfix

sysfix: sysfix.c $(FRC)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/sysfix sysfix.c  

stest:
	@set +e;if vax; then echo "\nNO SYSFIX on VAX\n";exit 1;fi

test:
	rtest $(TESTDIR)/sysfix

install: all
	$(INS) $(TESTDIR)/sysfix $(INSDIR)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/sysfix

FRC:

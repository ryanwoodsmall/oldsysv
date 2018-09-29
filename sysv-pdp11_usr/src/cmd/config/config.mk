# @(#)config.mk	1.2
TESTDIR = .
INS = /etc/install -n /usr/bin
INSDIR =
FRC =
CFLAGS = -O
LDFLAGS= -s
all: config
config:: config.vax.c
	-if vax; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config config.vax.c; fi
config:: config.pdp.c
	-if pdp11; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config config.pdp.c; fi
install: all
	$(INS) $(TESTDIR)/config
clean:
	rm -f *.o
clobber: clean
	rm -f $(TESTDIR)/config

FRC:

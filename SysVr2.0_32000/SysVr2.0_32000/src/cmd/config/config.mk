# @(#)config.mk	1.4
TESTDIR = .
INS = /etc/install -n /usr/bin
INSDIR =
FRC =
CFLAGS = -O
LDFLAGS= -s
all: config
config: 
	if vax; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config ./vax/config.c; fi
	if ns32000; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config ./ns32000/config.c; fi
	if pdp11; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config ./pdp11/config.c; fi
	if u3b; then $(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/config ./u3b20/config.c; fi

install: all
	$(INS) $(TESTDIR)/config
clean:
	rm -f *.o
clobber: clean
	rm -f $(TESTDIR)/config

FRC:

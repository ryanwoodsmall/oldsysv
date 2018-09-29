#	@(#)mkfs.mk	1.2
TESTDIR = .
FRC =
INS = /etc/install
IFLAG = 
LDFLAGS = -s 
INSDIR = -n /etc
CFLAGS = -O
FFLAG =
SMFLAG=

all: mkfs

mkfs: mkfs.c 
	-if pdp11 ; \
	then \
	$(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c; \
	else \
	$(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c; \
	$(CC) $(LDFLAGS) -DFsTYPE=1 $(CFLAGS) $(IFLAG) -o $(TESTDIR)/omkfs mkfs.c; \
	fi

install: all
	-if pdp11 ; \
	then \
	$(INS) $(INSDIR) $(TESTDIR)/mkfs /etc; \
	else \
	$(INS) $(INSDIR) $(TESTDIR)/mkfs /etc; \
	$(INS) $(INSDIR) $(TESTDIR)/omkfs /etc; \
	fi

clean:

clobber: clean
	-if pdp11 ; \
	then \
	rm -f $(TESTDIR)/mkfs; \
	else \
	rm -f $(TESTDIR)/mkfs; \
	rm -f $(TESTDIR)/omkfs; \
	fi

FRC:

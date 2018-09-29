#	@(#)mkfs.mk	1.5
ROOT =
TESTDIR = .
INCRT = $(ROOT)/usr/include
FRC =
INS = install
IFLAG =
LDFLAGS =
INSDIR = -n $(ROOT)/etc
CFLAGS = -O -s
FFLAG =
SMFLAG=

all: mkfs

mkfs: mkfs.c 
	-if ns32000 ; \
	then \
	$(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c; \
	else \
	$(CC) -I$(INCRT) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c; \
	$(CC) -I$(INCRT) $(LDFLAGS) -DFsTYPE=1 $(CFLAGS) $(IFLAG) -o $(TESTDIR)/omkfs mkfs.c; \
	fi

install: all
	-if ns32000 ; \
	then \
	$(INS) $(INSDIR) $(TESTDIR)/mkfs $(ROOT)/etc; \
	else \
	$(INS) $(INSDIR) $(TESTDIR)/mkfs $(ROOT)/etc; \
	$(INS) $(INSDIR) $(TESTDIR)/omkfs $(ROOT)/etc; \
	fi

clean:

clobber: clean
	-if ns32000 ; \
	then \
	rm -f $(TESTDIR)/mkfs; \
	else \
	rm -f $(TESTDIR)/mkfs; \
	rm -f $(TESTDIR)/omkfs; \
	fi

FRC:

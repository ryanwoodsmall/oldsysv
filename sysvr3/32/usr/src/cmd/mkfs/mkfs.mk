#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mkfs:mkfs.mk	1.7"
ROOT =
TESTDIR = .
INCRT = $(ROOT)/usr/include
FRC =
INS = install
IFLAG =
LDFLAGS = -s
INSDIR = $(ROOT)/etc
CFLAGS = -O -I$(INCRT)
FFLAG =
SMFLAG=

all: mkfs

mkfs: mkfs.c 
	-if pdp11 ; \
	then \
	$(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c $(LDLIBS); \
	else \
	$(CC) $(LDFLAGS) -DFsTYPE=1 $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs512 mkfs.c $(LDLIBS); \
	$(CC) $(LDFLAGS) -DFsTYPE=2 $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c $(LDLIBS); \
	if u3b2 ; \
	then \
	$(CC) $(LDFLAGS) -DFsTYPE=4 $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs2K mkfs.c $(LDLIBS); \
	elif u3b15 ; \
	then \
	$(CC) $(LDFLAGS) -DFsTYPE=4 $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs1K mkfs.c $(LDLIBS); \
	fi ; \
	fi

install: all
	-if pdp11 ; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/mkfs; \
	else \
	$(INS) -f $(INSDIR) $(TESTDIR)/mkfs; \
	$(INS) -f $(INSDIR) $(TESTDIR)/mkfs512; \
	if u3b2 ; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/mkfs2K; \
	elif u3b15 ; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/mkfs1K; \
	fi ; \
	fi

clean:
	rm -f *.o

clobber: clean
	-if pdp11 ; \
	then \
	rm -f $(TESTDIR)/mkfs; \
	else \
	rm -f $(TESTDIR)/mkfs $(TESTDIR)/mkfs512; \
	if u3b2 ; \
	then \
	rm -f $(TESTDIR)/mkfs2K; \
	elif u3b15 ; \
	then \
	rm -f $(TESTDIR)/mkfs1K; \
	fi ; \
	fi

FRC:

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mkfs:mkfs.mk	1.6"
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
	-if pdp11 ; \
	then \
	$(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c $(LDLIBS); \
	else \
	$(CC) -I$(INCRT) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/mkfs mkfs.c $(LDLIBS); \
	$(CC) -I$(INCRT) $(LDFLAGS) -DFsTYPE=1 $(CFLAGS) $(IFLAG) -o $(TESTDIR)/omkfs mkfs.c $(LDLIBS); \
	fi

install: all
	-if pdp11 ; \
	then \
	$(INS) $(INSDIR) $(TESTDIR)/mkfs $(ROOT)/etc; \
	else \
	$(INS) $(INSDIR) $(TESTDIR)/mkfs $(ROOT)/etc; \
	$(INS) $(INSDIR) $(TESTDIR)/omkfs $(ROOT)/etc; \
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

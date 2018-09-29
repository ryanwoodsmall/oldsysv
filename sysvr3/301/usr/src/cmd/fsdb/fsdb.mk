#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fsdb:fsdb.mk	1.7"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc
INC = $(ROOT)/usr/include
INS = :
CFLAGS = -s -O -D
FRC =

all: fsdb
fsdb::\
	$(ROOT)/usr/include/stdio.h\
	$(INC)/sys/param.h\
	$(ROOT)/usr/include/signal.h\
	$(INC)/sys/types.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/filsys.h\
	$(INC)/sys/dir.h\
	$(INC)/sys/ino.h\
	fsdb.c
	-if pdp11; \
	then \
		$(CC) $(CFLAGS)FsTYPE=1 -I$(INC) -o $(TESTDIR)/fsdb fsdb.c $(LDLIBS);\
	else \
		$(CC) $(CFLAGS)FsTYPE=2 -I$(INC) -o $(TESTDIR)/fsdb fsdb.c $(LDLIBS);\
		$(CC) $(CFLAGS)FsTYPE=1 -I$(INC) -o $(TESTDIR)/fsdb1b fsdb.c $(LDLIBS); \
	fi

fsdb::
	-if pdp11; \
	then \
		$(INS) -n $(INSDIR) $(TESTDIR)/fsdb ; \
	else \
		$(INS) -n $(INSDIR) $(TESTDIR)/fsdb ; \
		$(INS) -n $(INSDIR) $(TESTDIR)/fsdb1b ; \
	fi

install:
	make -f fsdb.mk INS=install FRC="$(FRC)" TESTDIR="$(TESTDIR)" \
		INSDIR="$(INSDIR)" INCRT="$(INCRT)" CFLAGS="$(CFLAGS)"

clean:

clobber : clean
	rm -f $(TESTDIR)/fsdb $(TESTDIR)/fsdb1b

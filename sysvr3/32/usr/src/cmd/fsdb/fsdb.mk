#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fsdb:fsdb.mk	1.11"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc
INC = $(ROOT)/usr/include
INS = install
CFLAGS = -O -I$(INC) -D
LDFLAGS = -s
FRC =

all:
	-if u3b2; then make -f fsdb.mk fsdb512 fsdb1K fsdb2K; \
	elif u3b15; then make -f fsdb.mk fsdb1b fsdb2b fsdb; \
	else make -f fsdb.mk fsdb1b fsdb; fi

fsdb512 fsdb1b::\
	$(ROOT)/usr/include/stdio.h\
	$(INC)/sys/param.h\
	$(ROOT)/usr/include/signal.h\
	$(INC)/sys/types.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/filsys.h\
	$(INC)/sys/dir.h\
	$(INC)/sys/ino.h\
	fsdb.c
	-if u3b2; \
	then \
	$(CC) $(LDFLAGS) $(CFLAGS)FsTYPE=1 -o $(TESTDIR)/fsdb512 fsdb.c $(LDLIBS);\
	else \
	$(CC) $(LDFLAGS) $(CFLAGS)FsTYPE=1 -o $(TESTDIR)/fsdb1b fsdb.c $(LDLIBS);\
	fi


fsdb1K fsdb::\
	$(ROOT)/usr/include/stdio.h\
	$(INC)/sys/param.h\
	$(ROOT)/usr/include/signal.h\
	$(INC)/sys/types.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/filsys.h\
	$(INC)/sys/dir.h\
	$(INC)/sys/ino.h\
	fsdb.c
	-if u3b2; \
	then \
	$(CC) $(LDFLAGS) $(CFLAGS)FsTYPE=2 -o $(TESTDIR)/fsdb1K fsdb.c $(LDLIBS);\
	else \
	$(CC) $(LDFLAGS) $(CFLAGS)FsTYPE=2 -o $(TESTDIR)/fsdb fsdb.c $(LDLIBS);\
	fi


fsdb2K fsdb2b::\
	$(ROOT)/usr/include/stdio.h\
	$(INC)/sys/param.h\
	$(ROOT)/usr/include/signal.h\
	$(INC)/sys/types.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/filsys.h\
	$(INC)/sys/dir.h\
	$(INC)/sys/ino.h\
	fsdb.c
	-if u3b2; \
	then \
	$(CC) $(LDFLAGS) $(CFLAGS)FsTYPE=4 -o $(TESTDIR)/fsdb2K fsdb.c $(LDLIBS);\
	elif u3b15; \
	then \
	$(CC) $(LDFLAGS) $(CFLAGS)FsTYPE=4 -o $(TESTDIR)/fsdb2b fsdb.c $(LDLIBS);\
	fi

install: all
	-if u3b2; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsdb512; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsdb1K; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsdb2K; \
	ln $(INSDIR)/fsdb1K $(INSDIR)/fsdb; \
	elif u3b15; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsdb1b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsdb2b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsdb; \
	else \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsdb1b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsdb; \
	fi

clean:

clobber : clean
	-if u3b2; \
	then \
	rm -f $(TESTDIR)/fsdb512 $(TESTDIR)/fsdb1K $(TESTDIR)/fsdb2K; \
	elif u3b15; \
	then \
	rm -f $(TESTDIR)/fsdb1b $(TESTDIR)/fsdb2b $(TESTDIR)/fsdb; \
	else \
	rm -f $(TESTDIR)/fsdb1b $(TESTDIR)/fsdb; \
	fi

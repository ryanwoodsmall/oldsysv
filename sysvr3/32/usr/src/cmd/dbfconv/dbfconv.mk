#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dbfconv:dbfconv.mk	1.3"

ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/net/nls
INC = $(ROOT)/usr/include
INS = :
CFLAGS = -O -I$(INC)
LDFLAGS = -s
SOURCE = dbfconv.c nlsstr.c
OBJECT = dbfconv.o nlsstr.o
FRC =

all: dbfconv

dbfconv: $(OBJECT)
	$(CC) $(CFLAGS) -o $(TESTDIR)/dbfconv $(LDFLAGS) $(OBJECT)

install: all
	install -f $(INSDIR) $(TESTDIR)/dbfconv

clean:
	rm -f $(OBJECT)

clobber: clean
	rm -f $(TESTDIR)/dbfconv

FRC:


dbfconv.o: $(INC)/sys/param.h \
	$(INC)/sys/fs/s5dir.h \
	local.h \
	$(INC)/stdio.h

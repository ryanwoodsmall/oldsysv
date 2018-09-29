#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)hpio:hpio.mk	1.3"
ROOT =
TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/usr/bin 
LDFLAGS = -s -n

all: hpio

hpio: hpio.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/hpio hpio.c  

install: all
	$(INS) -n $(INSDIR) $(TESTDIR)/hpio

clobber:
	-rm -f $(TESTDIR)/hpio

FRC:


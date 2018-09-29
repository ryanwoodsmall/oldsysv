#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bdiff:bdiff.mk	1.5"
#
#

ROOT= 

BIN=$(ROOT)/usr/bin

CFLAGS=-c

LDFLAGS= -s -n -i

LIBS= -lPW

all:	bdiff

bdiff:	bdiff.o
	$(CC) $(LDFLAGS) -o bdiff bdiff.o $(LIBS) 

bdiff.o:	bdiff.c
	$(CC) $(CFLAGS) bdiff.c

install:	all
	rm -f $(BIN)/bdiff
	mv bdiff $(BIN)/bdiff
	$(CH)chmod 755 $(BIN)/bdiff
	$(CH)chgrp bin $(BIN)/bdiff
	$(CH)chown bin $(BIN)/bdiff

clean:
	-rm -f bdiff.o

clobber:	clean
	-rm -f bdiff

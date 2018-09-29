#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fsba:fsba.mk	1.1"
DIR = $(ROOT)/etc
INC = $(ROOT)/usr/include
LDFLAGS = -s
CFLAGS = -O -I$(INC)
INS = install

all: fsba

fsba: fsba.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o fsba fsba.o

install: all
	$(INS) -f $(DIR) fsba

clean:
	rm -f *.o

clobber: clean
	rm -f fsba

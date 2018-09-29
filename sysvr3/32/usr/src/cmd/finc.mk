#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)finc:finc.mk	1.1"
DIR = $(ROOT)/etc
INC = $(ROOT)/usr/include
LDFLAGS = -s
CFLAGS = -O -I$(INC)
INS = install

all: finc

finc: finc.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o finc finc.o

install: all
	$(INS) -f $(DIR) finc

clean:
	rm -f *.o

clobber: clean
	rm -f finc

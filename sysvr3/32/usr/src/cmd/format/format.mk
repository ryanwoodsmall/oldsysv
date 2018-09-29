#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)format:format.mk	1.1"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
INS = install
FRC =

all:	format

install: all
	$(INS) -f $(ROOT)/etc format

format:
	$(CC) $(CFLAGS) $(LDFLAGS) -o format format.c $(LDLIBS)

clean:
	-rm -f format.o

clobber: clean
	-rm -f format

FRC:

#
# Header dependencies
#

format: format.c \
	$(INC)/stdio.h \
	$(INC)/fcntl.h \
	$(INC)/errno.h \
	$(INC)/sys/extbus.h \
	$(FRC)

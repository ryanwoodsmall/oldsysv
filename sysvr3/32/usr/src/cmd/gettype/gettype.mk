#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)gettype:gettype.mk	1.1"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
INS = install
FRC =

all:	gettype

install: all
	$(INS) -f $(ROOT)/etc gettype

gettype:
	$(CC) $(CFLAGS) $(LDFLAGS) -o gettype gettype.c $(LDLIBS)

clean:
	-rm -f gettype.o

clobber: clean
	-rm -f gettype

FRC:

#
# Header dependencies
#

gettype: gettype.c \
	$(INC)/stdio.h \
	$(INC)/errno.h \
	$(INC)/sys/pump.h \
	$(INC)/sys/extbus.h \
	$(FRC)

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)prtconf:prtconf.mk	1.5.1.2"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
INS = install
FRC =

all:	prtconf

install: all
	$(INS) -f $(ROOT)/etc prtconf

prtconf:
	$(CC) $(CFLAGS) prtconf.c -o prtconf $(LDFLAGS) -lld -lxedt $(LDLIBS)

clean:
	rm -f prtconf.o 

clobber: clean
	rm -f prtconf

#
# Header dependencies
#

prtconf: prtconf.c \
	$(INC)/ctype.h \
	$(INC)/stdio.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/param.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/types.h \
	$(INC)/sys/signal.h \
	$(FRC)

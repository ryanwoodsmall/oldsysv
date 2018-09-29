#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)prtconf:prtconf.mk	1.5.1.1"
#

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
INS = install
FRC =

all: prtconf

install: prtconf
	install -n $(ROOT)/etc prtconf

prtconf:
	$(CC) $(CFLAGS) prtconf.c -lld -o prtconf $(LDFLAGS) $(LDLIBS)

clobber: clean
	rm -f prtconf prtconf.o prtconf.lint

clean:

lint:	prtconf.lint

prtconf.lint:	prtconf.c
	lint -Uvax -I$(INC) prtconf.c > prtconf.lint

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

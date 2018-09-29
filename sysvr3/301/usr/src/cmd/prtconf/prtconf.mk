#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)prtconf:prtconf.mk	1.5"
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT)
INS = install
FRC =

all: prtconf

install: prtconf
	install -n $(ROOT)/etc prtconf

prtconf:
	$(CC) $(CFLAGS) prtconf.c -lld -o prtconf $(LDLIBS)

clobber: clean

clean:
	rm -f prtconf prtconf.o prtconf.lint

lint:	prtconf.lint

prtconf.lint:	prtconf.c
	lint -Uvax -I$(INCRT) prtconf.c > prtconf.lint

#
# Header dependencies
#

prtconf: prtconf.c \
	$(INCRT)/ctype.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/edt.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/signal.h \
	$(FRC)

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ckauto:ckauto.mk	1.8"
#	(IH)ckauto.mk	1.1.2.3
#
#
#		Copyright 1984 AT&T
#

ROOT =
INCRT = $(ROOT)/usr/include
FRC =
CFLAGS = -O
LDFLAGS = -s

all: ckauto

install: ckauto
	install -n $(ROOT)/etc ckauto

ckauto:
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(INCRT) ckauto.c -lld -o ckauto $(LDLIBS)

clobber: clean

clean:
	rm -f ckauto ckauto.lint

lint:	ckauto.lint

ckauto.lint:	ckauto.c
	lint -Uvax -I$(INCRT) ckauto.c > ckauto.lint

FRC:

#
# Header dependencies
#

ckauto: ckauto.c \
	$(INCRT)/errno.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sys3b.h \
	$(FRC)

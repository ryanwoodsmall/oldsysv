#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)prof:prof.mk	1.8"
#
#
ROOT	=
VERSION	=
UBINDIR	= $(ROOT)/usr/bin
USRINC	= $(ROOT)/usr/$(VERSION)/include
LDFLAGS = -s

all:	prof.o
	if pdp11; then \
		$(CC) $(CFLAGS) $(LDFLAGS) prof.o -o prof; \
	else \
		$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) prof.o -lld -o prof; \
	fi

prof.o:
	$(CC) -c $(FFLAG) $(CFLAGS) prof.c; 

prof:	all $(USRINC)/stdio.h $(USRINC)/a.out.h $(USRINC)/sys/types.h \
	$(USRINC)/sys/param.h $(USRINC)/mon.h

install:	prof
	cp prof $(UBINDIR)/prof
	$(CH) chmod 775 $(UBINDIR)/prof
	$(CH) chgrp bin $(UBINDIR)/prof
	$(CH) chown bin $(UBINDIR)/prof

clean:
	rm -f prof.o

clobber: clean
	rm -f prof

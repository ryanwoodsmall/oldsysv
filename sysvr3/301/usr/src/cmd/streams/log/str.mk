#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-streams:log/str.mk	1.5"

INCRT = $(ROOT)/usr/include
USERBIN = $(ROOT)/usr/bin
CFLAGS = -O -s -I$(INCRT) -Du3b2

PRODUCTS = strace strerr strclean

.c:
	$(CC) $(CFLAGS) -o $* $*.c $(LDLIBS)

all: $(PRODUCTS)

install: all
	for n in $(PRODUCTS) ; do \
		install -f $(USERBIN) -m 0100 -u 0 -g 3 $$n ; \
	done
	
clean:
	

clobber: 
	-rm -f $(PRODUCTS)


	


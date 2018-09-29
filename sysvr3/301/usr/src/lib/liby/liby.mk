#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)liby:liby.mk	1.6"
ROOT=
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
LORDER=lorder

CC=cc
CFLAGS=

LIBOBJS=libmai.o libzer.o

all:     $(USRLIB)/liby.a

$(USRLIB)/liby.a: $(LIBOBJS)
	ar rv tmplib.a `$(LORDER) *.o | tsort`;

libmai.o:	libmai.c
		$(CC) -c $(CFLAGS) libmai.c

libzer.o:	libzer.c
		$(CC) -c $(CFLAGS) libzer.c

install:  all
	cp tmplib.a $(USRLIB)/liby.a;

clean:
	-rm -f *.o

clobber:	clean
	-rm -f tmplib.a 

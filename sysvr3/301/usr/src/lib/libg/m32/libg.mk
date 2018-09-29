#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libg:m32/libg.mk	1.12"
ROOT=
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
all:	libg.a

libg.a: libg.s
	$(PFX)as -m libg.s
	mv libg.o libg.a

install: all
	cp libg.a $(USRLIB)/libg.a

clean:
	rm -f *.o

clobber: clean
	rm -f libg.a
	

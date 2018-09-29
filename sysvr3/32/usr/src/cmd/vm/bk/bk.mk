#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.bk:src/bk/bk.mk	1.4"
#

INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBRARY=libbk.a
HEADER1=../inc
INCLUDE= -I$(HEADER1)

AR=	ar
CFLAGS=	-O


$(LIBRARY):	\
			$(LIBRARY)(background.o) 

$(LIBRARY)(background.o): background.c  $(HEADER1)/wish.h


.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

##### Standard makefile targets ######

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)

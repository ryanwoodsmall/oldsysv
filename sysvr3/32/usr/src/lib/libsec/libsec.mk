#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libsec:libsec.mk	1.1"

USRLIB=$(ROOT)/usr/lib
INC=$(ROOT)/usr/include
INS=install
LIBRARY=libsec.a

all:	$(LIBRARY)

$(LIBRARY):	$(LIBRARY)(getspent.o) $(LIBRARY)(lckpwdf.o)

$(LIBRARY)(getspent.o):	getspent.c $(INC)/sys/param.h	\
			$(INC)/stdio.h $(INC)/shadow.h	\
			$(INC)/string.h $(INC)/sys/types.h

$(LIBRARY)(lckpwdf.o):	lckpwdf.c $(INC)/stdio.h	\
			$(INC)/signal.h $(INC)/fcntl.h

install:	all
	$(INS) -f $(USRLIB) -m 664 -u bin -g bin $(LIBRARY)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBRARY)

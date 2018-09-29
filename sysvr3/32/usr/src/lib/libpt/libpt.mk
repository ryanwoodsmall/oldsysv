#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libpt:libpt.mk	10.5"
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.
# 

ROOT=
USRLIB=$(ROOT)/usr/lib
ARFLAGS = rvu
INC=$(ROOT)/usr/include
CFLAGS= -I$(INC)

LIBOBJS= libpt.o pt_chmod.o

#
# Header dependencies
#

INCLUDE= $(INC)/sys/types.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/param.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/stropts.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/user.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/fcntl.h\
	$(INC)/sys/stat.h\
	$(INC)/sys/ptms.h

all:
	$(CC) -c $(CFLAGS) libpt.c 
	$(AR) $(ARFLAGS) libpt.a libpt.o
	$(CC) $(CFLAGS) pt_chmod.c libpt.a -o pt_chmod

$(LIBOBJS):	$(INCLUDES)

install:  all
	install -f $(USRLIB) libpt.a;
	install -f $(USRLIB) pt_chmod
	$(CH)chown bin $(USRLIB)/libpt.a
	$(CH)chgrp bin $(USRLIB)/libpt.a
	$(CH)chmod 664 $(USRLIB)/libpt.a
	$(CH)chgrp bin  $(USRLIB)/pt_chmod
	$(CH)chmod 04111 $(USRLIB)/pt_chmod
	$(CH)chown root $(USRLIB)/pt_chmod

clean:
	-rm -f *.o

clobber:	clean
	-rm -f libpt.a
	-rm -f pt_chmod


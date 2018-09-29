#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libxedt:libxedt.mk	1.1"

ROOT =
INC = $(ROOT)/usr/include
LIB = $(ROOT)/lib
USRLIB = $(ROOT)/usr/lib
CFLAGS = -O -I$(INC)
LDFLAGS = 
INS = install

all:	libxedt.a

libxedt.a: libxedt.o
	mv libxedt.o libxedt.a

install: all
	$(INS) -f $(USRLIB) libxedt.a 

clean:
	rm -f *.o

clobber: clean
	rm -f libxedt.a

libxedt.o: libxedt.c \
	$(INC)/ctype.h \
	$(INC)/fcntl.h \
	$(INC)/stdio.h \
	$(INC)/sys/edt.h \
	$(INC)/sys/extbus.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/libxedt.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vtoc.h \
	$(FRC)


#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)prtvtoc:prtvtoc.mk	1.6.3.2"
ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
INS = install
FRC =

all:	 prtvtoc

install: all
	$(INS) -f $(ROOT)/etc prtvtoc

prtvtoc:
	$(CC) $(CFLAGS) $(LDFLAGS) -o prtvtoc prtvtoc.c $(LDLIBS)

clean:
	-rm -f prtvtoc.o

clobber: clean
	-rm -f prtvtoc

FRC:

#
# Header dependencies
#

prtvtoc: prtvtoc.c \
	$(INC)/errno.h \
	$(INC)/fcntl.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vtoc.h \
	$(FRC)

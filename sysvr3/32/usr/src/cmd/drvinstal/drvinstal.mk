#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)drvinstal:drvinstal.mk	1.7"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS =-O -I$(INC)
LDFLAGS=-s
INS = install
FRC =

all: drvinstall 

install: all
		$(INS) -n $(ROOT)/etc drvinstall

drvinstall:
		$(CC) $(CFLAGS) -o drvinstall drvinstal.c $(LDFLAGS) $(LDLIBS)

clean:

clobber:	clean
		rm -f drvinstall 

FRC:

#
# Header dependencies
#

drvinstall: drvinstal.c \
	$(INC)/ctype.h \
	$(INC)/fcntl.h \
	$(INC)/filehdr.h \
	$(INC)/stdio.h \
	$(INC)/sys/dir.h \
	$(INC)/sys/param.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/types.h \
	$(FRC)

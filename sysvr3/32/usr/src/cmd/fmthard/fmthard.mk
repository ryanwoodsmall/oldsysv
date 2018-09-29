#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fmthard:fmthard.mk	1.4.2.2"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
INS = install
FRC =

all:	fmthard

install: all
	$(INS) -f $(ROOT)/etc fmthard

fmthard:
	$(CC) $(CFLAGS) $(LDFLAGS) -o fmthard fmthard.c $(LDLIBS)

clean:
	-rm -f fmthard.o

clobber: clean
	-rm -f fmthard

FRC:

#
# Header dependencies
#

fmthard: fmthard.c \
	$(INC)/fcntl.h \
	$(INC)/stdio.h \
	$(INC)/sys/id.h \
	$(INC)/sys/open.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vtoc.h \
	$(FRC)

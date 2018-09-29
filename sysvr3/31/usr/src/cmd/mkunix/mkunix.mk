#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mkunix:mkunix.mk	1.6"
#
#		Copyright 1984 AT&T
#
#

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -s
INS = install
FRC =

all: mkunix

install: mkunix
	$(INS) -n $(ROOT)/etc mkunix

mkunix:
	$(CC) $(CFLAGS) mkunix.c -lld -o mkunix 

clobber: clean

clean:
	rm -f mkunix mkunix.lint

lint:	mkunix.lint

mkunix.lint:	mkunix.c
	lint -Du3b2 -Uvax -I$(INC) mkunix.c > mkunix.lint

FRC:

#
# Header dependencies
#

mkunix: mkunix.c \
	$(INC)/a.out.h \
	$(INC)/aouthdr.h \
	$(INC)/fcntl.h \
	$(INC)/filehdr.h \
	$(INC)/ldfcn.h \
	$(INC)/linenum.h \
	$(INC)/nlist.h \
	$(INC)/reloc.h \
	$(INC)/scnhdr.h \
	$(INC)/stdio.h \
	$(INC)/storclass.h \
	$(INC)/syms.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sys3b.h \
	$(INC)/sys/types.h \
	$(FRC)

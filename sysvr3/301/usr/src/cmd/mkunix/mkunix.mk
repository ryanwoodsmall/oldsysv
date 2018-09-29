#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mkunix:mkunix.mk	1.5"
#
#		Copyright 1984 AT&T
#
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -s
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
	lint -Du3b2 -Uvax -I$(ROOT)/usr/include mkunix.c > mkunix.lint

FRC:

#
# Header dependencies
#

mkunix: mkunix.c \
	$(INCRT)/a.out.h \
	$(INCRT)/aouthdr.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/filehdr.h \
	$(INCRT)/ldfcn.h \
	$(INCRT)/linenum.h \
	$(INCRT)/nlist.h \
	$(INCRT)/reloc.h \
	$(INCRT)/scnhdr.h \
	$(INCRT)/stdio.h \
	$(INCRT)/storclass.h \
	$(INCRT)/syms.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/types.h \
	$(FRC)

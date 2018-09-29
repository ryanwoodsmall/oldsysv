#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)newboot:newboot.mk	1.3"
#
#		Copyright 1984 AT&T
#

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install
FRC =

all:	newboot

newboot:
	$(CC) $(CFLAGS) $(LDFLAGS) -o newboot newboot.c $(LDLIBS)

install: all
	$(INS) -n $(ROOT)/etc newboot

clean:
	-rm -f newboot.o

clobber: clean
	-rm -f newboot

FRC:

#
# Header dependencies
#

newboot: newboot.c \
	$(INCRT)/a.out.h \
	$(INCRT)/aouthdr.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/filehdr.h \
	$(INCRT)/linenum.h \
	$(INCRT)/nlist.h \
	$(INCRT)/reloc.h \
	$(INCRT)/scnhdr.h \
	$(INCRT)/stdio.h \
	$(INCRT)/storclass.h \
	$(INCRT)/syms.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)

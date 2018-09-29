#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)prtvtoc:prtvtoc.mk	1.6"
ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -Uu3b -Uvax -Updp11 -Uu3b5 -Du3b2
LDFLAGS = -s
INS = install
FRC =

all:	 prtvtoc

install: all
	$(INS) -n $(ROOT)/etc prtvtoc

prtvtoc:
	$(CC) $(CFLAGS) $(LDFLAGS) -o prtvtoc prtvtoc.c 

clean:
	-rm -f prtvtoc.o

clobber: clean
	-rm -f prtvtoc

FRC:

#
# Header dependencies
#

prtvtoc: prtvtoc.c \
	$(INCRT)/errno.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)

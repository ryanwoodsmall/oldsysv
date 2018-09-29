#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)drvinstal:drvinstal.mk	1.5"
########
#
#	drvinstall makefile for 3b2
#
########

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -s -O -I$(INCRT)
INS = install
FRC =

all: drvinstall 

install: all
		$(INS) -n $(ROOT)/etc drvinstall

drvinstall:
		$(CC) $(CFLAGS) -o drvinstall drvinstal.c $(LDLIBS)

clean:

clobber:	clean
		rm -f drvinstall 

FRC:

#
# Header dependencies
#

drvinstall: drvinstal.c \
	$(INCRT)/ctype.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/filehdr.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/types.h \
	$(FRC)

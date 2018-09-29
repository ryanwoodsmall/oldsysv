#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)getmajor:getmajor.mk	1.3"
# (IH)getmajor.mk	1.1
#
#		Copyright 1984 AT&T
#
########
#
#	getmajor makefile for 3b5
#
########

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -s -O -I$(INCRT)
INS = install
FRC =

all:	getmajor 

install: all
	$(INS) -n $(ROOT)/etc getmajor

getmajor:
	$(CC) $(CFLAGS) -o getmajor getmajor.c  $(LDLIBS)

clean:
	rm -f getmajor.o

clobber: clean
	rm -f getmajor

FRC:

#
# Header dependencies
#

getmajor: getmajor.c \
	$(INCRT)/ctype.h \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/edt.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sys3b.h \
	$(INCRT)/sys/types.h \
	$(FRC)

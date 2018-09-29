#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fltboot:fltboot.mk	1.5.1.1"
ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -I$(INC)
INS = install
INSDIR = $(ROOT)/etc

LDFLAGS = -s
FRC =

all: fltboot

install: fltboot
	 $(INS) -f $(INSDIR) fltboot 

fltboot:	fltboot.c
		$(CC) $(CFLAGS) -o fltboot fltboot.c $(LDFLAGS) $(LDLIBS)
clean:

clobber: clean
	rm -f fltboot


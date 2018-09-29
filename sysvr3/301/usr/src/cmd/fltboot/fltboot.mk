#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fltboot:fltboot.mk	1.5"
ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -I$(INCRT)
SIZE = $(SGS)size
DIS = $(SGS)dis
NM = $(SGS)nm
STRIP = $(SGS)strip
CONV = $(SGS)conv
INS = install

LDFLAG =
FRC =

all: fltboot

install: fltboot
	 cp fltboot $(ROOT)/etc/fltboot
	 $(STRIP) $(ROOT)/etc/fltboot

fltboot:		fltboot.c
			$(CC) $(CFLAGS) -o fltboot fltboot.c $(LDLIBS)
clean:
	rm -f fltboot

clobber: clean

debug: fltboot
	$(SIZE) fltboot > boot.size
	$(NM) -nef fltboot > fltboot.name
	$(STRIP) fltboot


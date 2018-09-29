#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:boot/mboot/makefile	10.2"

ROOT =
INCRT = ../..
INCLOC = ..
CFLAGS = -I$(INCLOC) -I$(INCRT)
SIZE = size
DIS = dis
NM = nm
STRIP = strip
CONV = conv
INS = install

LDFLAG =
FRC =

all: mboot

install: mboot
	$(STRIP) mboot
	$(INS) -f $(ROOT)/lib mboot

mboot: mbld\
	mboot.o\
	$(FRC)
	$(LD) $(LDFLAG) mbld -o mboot -lc

clean:
	rm -f mboot *.o

clobber: clean

debug: mboot
	$(SIZE) mboot > mboot.size
	$(DIS) -L mboot > mboot.dis
	$(NM) -nef mboot > mboot.name
	$(STRIP) mboot
	pr -n mboot.c mbld mboot.dis mboot.name mboot.size | opr -f hole -txr -p land

FRC:

#
# Header dependencies
#

mboot.o: mboot.c \
	$(INCRT)/sys/boot.h \
	$(INCRT)/sys/firmware.h \
	$(INCLOC)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vtoc.h \
	$(FRC)

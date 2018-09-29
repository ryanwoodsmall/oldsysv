#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)df:df.mk	1.12"

#	df $(MAKE) file

#	since df requires the devnm function, they have been combined.
#	during installation, devnm is linked to df

ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/bin
DVDIR = $(ROOT)/etc
CFLAGS = -O
LDFLAGS = -s 
INS=install
MAKE = make

all:	install clobber

df: df.c
	$(CC) -I$(INC) $(CFLAGS) $(LDFLAGS) -o df df.c $(LDLIBS)

devnm:	df

install: df
	$(INS) -f $(INSDIR) -m 4755 -u root -g bin df
	$(MAKE) -i -f df.mk dvins

dvins:	devnm
	ln $(INSDIR)/df $(DVDIR)/devnm

clean:
	-rm -f df.o

clobber: clean
	rm -f df

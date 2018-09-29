#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)attwin:cmd/layers/misc/makefile	1.5"
#
#		Copyright 1985 AT&T
#

CC = cc
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
INS = install
STRIP = strip

all:	ismpx jterm jwin

ismpx:	ismpx.c
ismpx:	$(INC)/stdio.h
ismpx:	$(INC)/sys/jioctl.h
	$(CC) $(CFLAGS) -o ismpx ismpx.c

jterm:	jterm.c
jterm:	$(INC)/sys/jioctl.h
	$(CC) $(CFLAGS) -o jterm jterm.c

jwin:	jwin.c
jwin:	$(INC)/sys/jioctl.h
	$(CC) $(CFLAGS) -o jwin jwin.c

install:	all
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 ismpx
	$(STRIP) $(ROOT)/usr/bin/ismpx
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 jterm
	$(STRIP) $(ROOT)/usr/bin/jterm
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 jwin
	$(STRIP) $(ROOT)/usr/bin/jwin

clean:

clobber:
	rm -f ismpx jterm jwin

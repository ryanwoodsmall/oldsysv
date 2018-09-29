#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)attwin:cmd/layers/misc/makefile	1.1"
#
#		Copyright 1985 AT&T
#

CFLAGS=
INC = /usr/include

all:	install

install:	ismpx jterm jwin
	mv ismpx jterm jwin /usr/bin

ismpx:	ismpx.c
ismpx:	$(INC)/stdio.h
ismpx:	$(INC)/sys/jioctl.h
	cc $(CFLAGS) -o ismpx ismpx.c

jterm:	jterm.c
jterm:	$(INC)/sys/jioctl.h
	cc $(CFLAGS) -o jterm jterm.c

jwin:	jwin.c
jwin:	$(INC)/sys/jioctl.h
	cc $(CFLAGS) -o jwin jwin.c

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)attwin:cmd/layers/makefile	1.1"
#
#		Copyright 1985 AT&T
#

INC = /usr/include

all:	install

install:	layers layersys/lsys.873 set_enc.j relogin xtd xts xtt
	cp layers xtd xts xtt /usr/bin
	chmod 4755 /usr/bin/layers
	chown root /usr/bin/layers
	cp layersys/lsys.873 /usr/lib/layersys/lsys.8\;7\;3
	cp relogin set_enc.j /usr/lib/layersys
	chmod 4755 /usr/lib/layersys/relogin
	chown root /usr/lib/layersys/relogin

layers:	layers.o xtraces.o xtstats.o
	cc $(LDFLAGS) -o layers layers.o xtraces.o xtstats.o -lagent

relogin:	relogin.c
relogin:	$(INC)/sys/types.h
relogin:	$(INC)/utmp.h
relogin:	$(INC)/stdio.h
relogin:	$(INC)/pwd.h
	cc $(CFLAGS) -o relogin relogin.c

xtd:	xtd.c
xtd:	$(INC)/sys/param.h
xtd:	$(INC)/sys/types.h
xtd:	$(INC)/sys/tty.h
xtd:	$(INC)/sys/jioctl.h
xtd:	$(INC)/sys/xtproto.h
xtd:	$(INC)/sys/xt.h
xtd:	$(INC)/stdio.h
	cc $(CFLAGS) -o xtd xtd.c

xts:	xts.o xtstats.o
	cc $(LDFLAGS) -o xts xts.o xtstats.o

xtt:	xtt.o xtraces.o
	cc $(LDFLAGS) -o xtt xtt.o xtraces.o

layers.o:	layers.c
layers.o:	$(INC)/sys/types.h
layers.o:	$(INC)/sys/stat.h
layers.o:	$(INC)/sys/errno.h
layers.o:	$(INC)/sys/tty.h
layers.o:	$(INC)/sys/jioctl.h
layers.o:	$(INC)/sys/xtproto.h
layers.o:	$(INC)/sys/xt.h
layers.o:	$(INC)/sys/termio.h
layers.o:	$(INC)/fcntl.h
layers.o:	$(INC)/signal.h
layers.o:	$(INC)/stdio.h
layers.o:	sxtstat.h

xts.o:	xts.c
xts.o:	$(INC)/stdio.h

xtt.o:	xtt.c
xtt.o:	$(INC)/sys/types.h
xtt.o:	$(INC)/sys/tty.h
xtt.o:	$(INC)/sys/jioctl.h
xtt.o:	$(INC)/sys/xtproto.h
xtt.o:	$(INC)/sys/xt.h
xtt.o:	$(INC)/stdio.h

xtraces.o:	xtraces.c
xtraces.o:	$(INC)/stdio.h
xtraces.o:	$(INC)/sys/param.h
xtraces.o:	$(INC)/sys/types.h
xtraces.o:	$(INC)/sys/tty.h
xtraces.o:	$(INC)/sys/jioctl.h
xtraces.o:	$(INC)/sys/xtproto.h
xtraces.o:	$(INC)/sys/xt.h

xtstats.o:	xtstats.c
xtstats.o:	$(INC)/sys/types.h
xtstats.o:	$(INC)/sys/tty.h
xtstats.o:	$(INC)/sys/jioctl.h
xtstats.o:	$(INC)/sys/xtproto.h
xtstats.o:	$(INC)/sys/xt.h
xtstats.o:	$(INC)/stdio.h

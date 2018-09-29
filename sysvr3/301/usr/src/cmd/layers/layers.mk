#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)attwin:cmd/layers/makefile	1.12"
#
#		Copyright 1985 AT&T
#

CC = cc
INC = $(ROOT)/usr/include
LIB = $(ROOT)/usr/lib
CFLAGS = -O -I$(INC)
INS = install
STRIP = strip

all:	xt_driver wtinit_mk misc_mk libwindows_mk layers layersys/lsys.873 set_enc.j relogin xtd xts xtt 

layers:	libwindows_mk layers.o xtraces.o xtstats.o
	$(CC) $(CFLAGS) -o layers layers.o xtraces.o xtstats.o ../../lib/libwindows/libwindows.a

relogin:	relogin.c
relogin:	$(INC)/sys/types.h
relogin:	$(INC)/utmp.h
relogin:	$(INC)/stdio.h
relogin:	$(INC)/pwd.h
	$(CC) $(CFLAGS) -o relogin relogin.c

xtd:	xtd.c
xtd:	$(INC)/sys/param.h
xtd:	$(INC)/sys/types.h
xtd:	$(INC)/sys/tty.h
xtd:	$(INC)/sys/jioctl.h
xtd:	$(INC)/sys/xtproto.h
xtd:	$(INC)/sys/xt.h
xtd:	$(INC)/stdio.h
	$(CC) $(CFLAGS) -o xtd xtd.c

xts:	xts.o xtstats.o
	$(CC) $(CFLAGS) -o xts xts.o xtstats.o

xtt:	xtt.o xtraces.o
	$(CC) $(CFLAGS) -o xtt xtt.o xtraces.o

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

xt_driver:
	cd ../../uts/3b2/io; $(MAKE) -f xt.mk

wtinit_mk:
	cd wtinit; $(MAKE) -f wtinit.mk

misc_mk:
	cd misc; $(MAKE) -f misc.mk

libwindows_mk:
	cd ../../lib/libwindows; $(MAKE) -f libwindows.mk

install:	all
	if [ ! -d $(ROOT)/usr/lib/layersys ] ;\
	then \
		mkdir $(ROOT)/usr/lib/layersys ;\
		$(CH)chmod 755 $(ROOT)/usr/lib/layersys ;\
		$(CH)chgrp bin $(ROOT)/usr/lib/layersys ;\
		$(CH)chown bin $(ROOT)/usr/lib/layersys ;\
	fi 
	$(INS) -f $(ROOT)/usr/bin -u root -g bin -m 4755 layers
	$(STRIP) $(ROOT)/usr/bin/layers
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 xtd
	$(STRIP) $(ROOT)/usr/bin/xtd
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 xts
	$(STRIP) $(ROOT)/usr/bin/xts
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 xtt
	$(STRIP) $(ROOT)/usr/bin/xtt
	$(INS) -f $(ROOT)/usr/lib/layersys -u root -g bin -m 4755 relogin
	$(STRIP) $(ROOT)/usr/lib/layersys/relogin
	cp layersys/lsys.873 $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(CH)chown bin $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(CH)chgrp bin $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(CH)chmod 755 $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(STRIP) $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(INS) -f $(ROOT)/usr/lib/layersys -u bin -g bin -m 755 set_enc.j
	$(STRIP) $(ROOT)/usr/lib/layersys/set_enc.j
	cd wtinit; $(MAKE) -f wtinit.mk install
	cd misc; $(MAKE) -f misc.mk install
	cd ../../lib/libwindows; $(MAKE) -f libwindows.mk install
	cd ../../uts/3b2/io; $(MAKE) -f xt.mk install

clean:
	rm -f layers.o xts.o xtt.o xtraces.o xtstats.o 
	cd wtinit; $(MAKE) -f wtinit.mk clean
	cd misc; $(MAKE) -f misc.mk clean
	cd ../../lib/libwindows; $(MAKE) -f libwindows.mk clean
	cd ../../uts/3b2/io; $(MAKE) -f xt.mk clean

clobber:
	rm -f layers.o xts.o xtt.o xtraces.o xtstats.o layers relogin xtd xts xtt
	cd wtinit; $(MAKE) -f wtinit.mk clobber
	cd misc; $(MAKE) -f misc.mk clobber
	cd ../../lib/libwindows; $(MAKE) -f libwindows.mk clobber
	cd ../../uts/3b2/io; $(MAKE) -f xt.mk clobber

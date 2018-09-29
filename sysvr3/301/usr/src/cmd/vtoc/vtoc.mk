#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vtoc:vtoc.mk	1.2"

ROOT =
INS = install

all: 	install

install:
	if [ ! -d $(ROOT)/etc/vtoc ];then mkdir $(ROOT)/etc/vtoc; fi;
	$(INS) -f $(ROOT)/etc/vtoc 	hd2dft;
	$(INS) -f $(ROOT)/etc/vtoc	hd3dft;
	$(INS) -f $(ROOT)/etc/vtoc	hd5dft;
	$(INS) -f $(ROOT)/etc/vtoc	hd6dft;
	$(INS) -f $(ROOT)/etc/vtoc	hd7dft;
	$(INS) -f $(ROOT)/etc/vtoc	hd8dft;
	$(INS) -f $(ROOT)/etc/vtoc	hd9dft;
	$(INS) -f $(ROOT)/etc/vtoc	hd10dft;
clean:

clobber:	clean


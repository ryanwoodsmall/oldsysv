#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfuadmin:rfuadmin.mk	1.2"

#	rfuadmin make file

ROOT=
INSDIR = $(ROOT)/usr/bin
INS=/etc/install

all:	install clobber

rfuadmin:
	sh rfuadmin.sh
	chmod 755 rfuadmin

install: rfuadmin
	$(INS) -f $(INSDIR) -m 755 -u root -g bin rfuadmin

clean:
	rm -f rfuadmin

clobber: clean

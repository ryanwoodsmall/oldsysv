#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dsconfig:dsconfig.mk	1.2"
ROOT =
INC = $(ROOT)/usr/include
INS = install
INSDIR = $(ROOT)/usr/bin
FRC =

all: dsconfig

install: all
	 $(INS) -f $(INSDIR) -m 555  dsconfig 

dsconfig: dsconfig.sh
	 cp dsconfig.sh dsconfig

clean:

clobber: clean
	-rm -f dsconfig


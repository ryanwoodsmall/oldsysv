#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:etc/Makefile	1.1"
#
# Makefile for lp/etc
#


INS	=	install


ETC	=	$(ROOT)/etc

INIT_D	=	$(ETC)/init.d

OWNER	=	root
GROUP	=	sys
RMODES	=	0444


all:

install:	all
	$(INS) -m $(RMODES) -u $(OWNER) -g $(GROUP) -f $(INIT_D) lp

clean:	

clobber:

strip:

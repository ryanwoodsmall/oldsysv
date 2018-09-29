#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-3b2:lbin.mk	1.2"
ROOT =
INS = install

all: 	install

install:
	$(INS) -f ${ROOT}/usr/lbin	askx
	$(INS) -f ${ROOT}/usr/lbin	ignore
	$(INS) -f ${ROOT}/usr/lbin	mklost+found
	$(INS) -f ${ROOT}/usr/lbin	optparttn
	$(INS) -f ${ROOT}/usr/lbin	rmkdir
	$(INS) -f ${ROOT}/usr/lbin	rrmdir

clear:

clobber:   clear


#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:terminfo/Makefile	1.3"
#
# Makefile for the Terminfo entries delivered with LP
#


USRLIB	=	$(ROOT)/usr/lib
TERMINFO=	$(USRLIB)/terminfo


SRCS	= \
		40.ti \
		435.ti \
		43ro.ti \
		442.ti \
		455.ti \
		457.ti \
		470.ti \
		474.ti \
		477.ti \
		477ibmc.ti \
		477ibmg.ti \
		477qume.ti \
		479.ti \
		495hp.ti \
		495ibm.ti \
		495qume.ti \
		5320.ti \
		unknown.ti

all:
		
install:	all
	for ti in $(SRCS); do TERMINFO=$(TERMINFO) tic -v $$ti; done

clean:	

clobber:	clean

strip:

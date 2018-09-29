#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-streams:streams.mk	1.3"

LDLIBS =
ROOT =


all:
	cd log ; $(MAKE) -f str.mk ROOT='$(ROOT)' LDLIBS='$(LDLIBS)' all

install:
	cd log ; $(MAKE) -f str.mk ROOT='$(ROOT)' LDLIBS='$(LDLIBS)' install

clobber:
	cd log ; $(MAKE) -f str.mk clobber

clean:
	cd log ; $(MAKE) -f str.mk clean


#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:makefile	1.17"
#
# makefile for libc/port
#
#
# The macro PROF is null by default, causing profiled object to be maintained.
# If profiled object is not desired, the reassignment PROF=@# should appear in
# the make command line.
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O
PROF=
NONPROF=
DEFLIST=
SDEFLIST=

all:	prof nonprof 

nonprof:
	$(NONPROF)cd gen; make -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd print; make -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd stdio; make -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd sys; make -e $(IGN) nonprof PROF=@#
prof:
	$(PROF)cd gen; make -e $(IGN) prof NONPROF=@#
	$(PROF)cd print; make -e $(IGN) prof NONPROF=@#
	$(PROF)cd stdio; make -e $(IGN) prof NONPROF=@#
	$(PROF)cd sys; make -e $(IGN) prof NONPROF=@#
	
clean:

clobber: clean
	-rm -f */*.o
	-rm -f */*.p

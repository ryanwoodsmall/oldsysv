#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:lib/Makefile	1.5"
#
# Makefile for LP private libraries
#


##########
#
# If you add directories other than one level deep
# you'll have to change the code below.
##########
LIBDIRS	= \
		./access \
		./class \
		./filters \
		./forms \
		./fs \
		./lp \
		./msgs \
		./oam \
		./requests \
		./printers \
		./users


all:		libs

install:
	@echo "Not a delivered part of LP; not installed"
	@false

clean:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) clean; cd ..; done

clobber:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) clobber; cd ..; done

strip:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) STRIP=$(STRIP) strip; cd ..; done

libs:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE); cd ..; done

prof:		plibs

plibs:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) prof; cd ..; done

lprof:		qlibs

qlibs:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) lprof; cd ..; done

mdl:		mlibs

mlibs:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) mdl; cd ..; done

lint:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) lint; cd ..; done

lintsrc:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) FUNCDLC="$(FUNCDCL)" lintsrc; cd ..; done

lintlib:
	for dir in $(LIBDIRS); do cd $$dir; $(MAKE) lintlib; cd ..; done

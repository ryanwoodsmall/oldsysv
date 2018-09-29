#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)fmli:makefile	1.4"
#


INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBCURSES=$(USRLIB)/libcurses.a
CURSES_H=$(ROOT)/usr/include
CFLAGS= -O
LDFLAGS= -s

DIRS =	sys proc vt menu qued form oh oeu wish xx 



all:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo Making $@ in $$d subsystem;\
		make -f $$d.mk INC="$(INC)" USRLIB="$(USRLIB)" LIBCURSES="$(LIBCURSES)" CURSES_H="$(CURSES_H)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $@;\
		cd ..;\
	done

install: all
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo Making $@ in $$d subsystem;\
		make -f $$d.mk INC="$(INC)" USRLIB="$(USRLIB)" LIBCURSES="$(LIBCURSES)" CURSES_H="$(CURSES_H)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $@;\
		cd ..;\
	done


clean:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		make -f $$d.mk clean $@;\
		cd ..;\
	done

clobber: clean
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		make -f $$d.mk clobber $@;\
		cd ..;\
	done

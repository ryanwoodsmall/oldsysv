#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)curses:libcurses.mk	1.11"
#
#	Curses Library High Level Makefile.
#
#	To INSTALL libcurses.a, the tic compiler and the tools type:
#
#		"make install"
#
#
#	To COMPILE libcurses.a, the tic compiler and the tools, type:
#
#		"make all"
#
#
#	To compile a particular file with normal compilation type:
#
#		"make FILES='<particular .o files>"
#
#
#	If debugging is desired then type:
#
#		"make O=debug FILES='<particular .o files>"
#
#
#	If tracing is desired then type:
#
#		"make O=trace FILES='<particular .o files>"
#
#
#	If profiling is desired then type:
#
#		"make O=profile FILES='<particular .o files>"
#
#
#	To compile only the tic compiler type:
#
#		"make tic"
#
#
#	To create cpio files for all directories type:
#		"make cpio"
#
#
SHELL=/bin/sh
MAKE=make
ROOT=
USRLIB = $(ROOT)/usr/lib

all:	libcurses.a tools

libcurses.a:
	@cd ./screen ; $(MAKE) rmhdrs
	@cd ./screen ; $(MAKE) cktmp
	@cd ./screen ; $(MAKE) libcurses.a
	@echo
	@echo "Libcurses.a has been made."
	@echo

tools:
	@cd ./screen ; $(MAKE) tools
	@cd ./screen ; $(MAKE) llib-lcurses.ln
	@echo
	@echo "Libcurses/Terminfo tools have been made."
	@echo

tic:
	@cd ./screen ; $(MAKE) tic
	@echo
	@echo "The tic compiler has been made."
	@echo
	
install:
	# make and install libcurses.a and tic
	@cd ./screen ; $(MAKE) cktmp
	@cd ./screen ; $(MAKE) install USRLIB=$(USRLIB) CC2=$(CC) ROOT2=""
	@echo
	@echo libcurses.a, the tic compiler, and associated tools have
	@echo been installed.
	@echo
	@if [ "$(USRLIB)" != "$(ROOT)/usr/lib" ]; \
	then \
		echo; \
		echo Now make native tic for compiling core entries; \
		echo; \
		cd ./screen; /bin/make CC=cc ticclob tic; \
		echo; \
		echo The tic in $(ROOT)/usr/bin is built for the current environment.; \
		echo The tic found in `pwd`/screen is built native for this processor.; \
		echo; \
		echo Install native tic in `echo ${PATH} | cut -d: -f1`; \
		echo; \
		cpset tic `echo ${PATH} | cut -d: -f1` 755 ; \
	fi

clean:
	@cd ./screen ; $(MAKE) clean

clobber:
	@cd ./screen ; $(MAKE) clobber

cpio:
	@echo
	@echo "\n\tBuilding cpio files in ${HOME}\n\n\t\c"
	@find . -print|cpio -ocud|split -10000 - ${HOME}/crs.`date +%m%d`.
	@echo "\n\tcpio files have been built\n"

bsd:
	@echo
	cd screen; mv makefile makefile.sysv; cp makefile.bsd makefile
	cd screen; make rmident
	@echo "Curses has been converted for BSD"
# this has only been tested on 4.2BSD, but we assume libc has getopt.

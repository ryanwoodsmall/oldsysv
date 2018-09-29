#	@(#)makefile	1.11
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

all:
	cd gen; make -e $(IGN) nonprof
	cd print; make -e $(IGN) nonprof
	cd stdio; make -e $(IGN) nonprof
	cd sys; make -e $(IGN) nonprof
	cd gen; $(PROF)make -e $(IGN) prof
	cd print; $(PROF)make -e $(IGN) prof
	cd stdio; $(PROF)make -e $(IGN) prof
	cd sys; $(PROF)make -e $(IGN) prof

clean:

clobber: clean
	-rm -f */*.o
	-rm -f */*.p

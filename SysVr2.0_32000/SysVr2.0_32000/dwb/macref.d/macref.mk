#	text subsystem macref make file
#
# DSL 2
#	SCCS: @(#)macref.mk	1.4

OL = $(ROOT)/
INS = :
CINSDIR = ${OL}usr/bin
LINSDIR = ${OL}usr/lib/macref
SFILES = macref.sh
FFILES = macref.sno macrform.sno macrstat.sno macrtoc.sno
FILES = ${SFILES} ${FFILES}
MAKE = make

compile all:  macref filters
	:

macref:
	cp macref.sh macref
	${INS} macref ${CINSDIR}
	cd ${CINSDIR}; chmod 755 macref; $(CH) chgrp bin macref; chown bin macref

filters:
	${INS} ${FFILES} ${LINSDIR}
	cd ${LINSDIR}; chmod 644 ${FFILES};  $(CH) chgrp bin ${FFILES}; chown bin ${FFILES}

ldir:
	if [ ! -d ${LINSDIR} ]; \
	    then mkdir ${LINSDIR}; \
	fi

install:	ldir
	${MAKE} -f macref.mk INS=cp ROOT=$(ROOT) CH=$(CH)

clean:
	:
clobber:  clean macrefclobber
	:
macrefclobber:  ;  rm -f macref

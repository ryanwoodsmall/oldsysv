#	makefile for man command and term help message.
#
# DSL 2.
#	SCCS:  @(#)man.mk	1.3

OL = $(ROOT)/
INS = :
CINSDIR = ${OL}usr/bin
HINSDIR = ${OL}usr/lib/help
FILES = man.sh term
MAKE = make

compile all:  man termh
	:

man:
	cp man.sh man
	${INS} man ${CINSDIR}
	cd ${CINSDIR}; chmod 755 man; $(CH) chgrp bin man; chown bin man

helpdir:
	-mkdir ${OL}usr/lib/help

termh:	helpdir
	${INS} term ${HINSDIR}
	cd ${HINSDIR}; chmod 664 term; $(CH) chgrp bin term; chown bin term


install:
	${MAKE} -f man.mk INS=cp ROOT=$(ROOT) CH=$(CH)
insman:	;  ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) man
instermh: ; ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) termh

clean manclean:
	:
clobber:  clean
	rm -f man

manclobber:  ;  rm -f man

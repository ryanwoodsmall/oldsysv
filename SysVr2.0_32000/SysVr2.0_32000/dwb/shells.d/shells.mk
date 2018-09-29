#	text subsystem shells make file
#
# DSL 2.
#	SCCS:  @(#)shells.mk	1.24

OL = $(ROOT)/
INS = :
CINSDIR = ${OL}usr/bin
HINSDIR = ${OL}usr/lib/help
FILES = mm.sh mmt.sh man.sh osdd.sh text
MAKE = make

compile all:  man mm mmt mvt osdd texth
	:

man:	;  cp man.sh man
	${INS} man ${CINSDIR}
	cd ${CINSDIR}; chmod 755 man; $(CH) chgrp bin man; chown bin man

mm:	;  cp mm.sh mm
	${INS} mm ${CINSDIR}
	cd ${CINSDIR}; chmod 755 mm; $(CH) chgrp bin mm; chown bin mm

mmt:	;  cp mmt.sh mmt
	${INS} mmt ${CINSDIR}
	cd ${CINSDIR}; chmod 755 mmt; $(CH) chgrp bin mmt; chown bin mmt

mvt:	mmt
	rm -f ${CINSDIR}/mvt
	ln ${CINSDIR}/mmt ${CINSDIR}/mvt
	cd ${CINSDIR}; chmod 755 mvt; $(CH) chgrp bin mvt; chown bin mvt

osdd:	;  cp osdd.sh osdd
	   ${INS} osdd ${CINSDIR}
	cd ${CINSDIR}; chmod 755 osdd; $(CH) chgrp bin osdd; chown bin osdd

helpdir:
	-mkdir ${OL}usr/lib/help

texth:	helpdir
	${INS} text ${HINSDIR}
	cd ${HINSDIR}; chmod 644 text; $(CH) chgrp bin text; chown bin text

install:
	${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH)
insmm:	;  ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) mm
insmmt:	;  ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) mmt
insman:	;  ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) man
insmvt:	;  ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) mvt
insosdd: ; ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) osdd
instexth: ; ${MAKE} -f shells.mk INS=cp ROOT=$(ROOT) CH=$(CH) texth

clean manclean mmclean mmtclean mvtclean osddclean:
	:
clobber:  clean manclobber mmclobber mmtclobber orgclobber \
		osddclobber
	:
manclobber:  ;  rm -f man
mmclobber:   ;  rm -f mm
mmtclobber mvtclobber:  ;  rm -f mmt
orgclobber:  ;  rm -f org
osddclobber: ;  rm -f osdd

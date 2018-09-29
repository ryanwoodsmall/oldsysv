#	nroff/otroff make file (text subsystem)
#
# DSL 2
#	SCCS:	@(#)roff.mk	1.25

OL = $(ROOT)/
INS = :
ARGS = all
CINSDIR = ${OL}usr/bin
DINSDIR = ${OL}usr/lib
MAKE = make

compile all:  nroff suftab term otroff font
	:

nroff:
	if pdp11; \
	   then ${MAKE} nroff clean; \
	   else ${MAKE} nroff clean OPTS=-DINCORE; \
	fi
	${INS} nroff ${CINSDIR}
	cd ${CINSDIR}; chmod 755 nroff; $(CH) chgrp bin nroff; chown bin nroff

otroff:
	if pdp11; \
	   then ${MAKE} PTAG= otroff clean; \
	   else ${MAKE} PTAG= otroff clean OPTS=-DINCORE; \
	fi
	${INS} otroff ${CINSDIR}
	cd ${CINSDIR}; chmod 755 otroff; $(CH) chgrp bin otroff; chown bin otroff

suftab:
	${MAKE} suftab
	${INS} suftab ${DINSDIR}
	cd ${DINSDIR}; chmod 644 suftab; $(CH) chgrp bin suftab; chown bin suftab

term:
	cd terms.d; ${MAKE} -f terms.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}
font:
	cd fonts.d; ${MAKE} -f fonts.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

install:
	${MAKE} -f roff.mk INS=cp ROOT=$(ROOT) CH=$(CH) ARGS=install ${ARGS}
insnroff:  ;  ${MAKE} -f roff.mk INS=cp ROOT=$(ROOT) CH=$(CH) nroff
insotroff:  ;  ${MAKE} -f roff.mk INS=cp ROOT=$(ROOT) CH=$(CH) otroff
insuftab:  ;  ${MAKE} -f roff.mk INS=cp ROOT=$(ROOT) CH=$(CH) suftab

clean:	nclean
	cd terms.d; ${MAKE} -f terms.mk clean
	cd fonts.d; ${MAKE} -f fonts.mk clean
nclean:	;  ${MAKE} clean

clobber: nclobber
	cd terms.d; ${MAKE} -f terms.mk clobber
	cd fonts.d; ${MAKE} -f fonts.mk clobber
nclobber: ;  ${MAKE} clobber
bclobber: ;  ${MAKE} bclobber

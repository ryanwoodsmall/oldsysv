#	text sub-system make file
#
# DSL 2.
#	SCCS:  @(#)text.mk	1.21

OL = $(ROOT)/
ARGS = all
MAKE = make

compile all:	roff troff macros shells eqn neqn tbl checkkmm pic old macref mmlint sroff x9700
	:

roff:	;  cd roff.d;  ${MAKE} -f roff.mk ROOT=$(ROOT) CH=$(CH) ${ARGS} clean

troff:	;  cd troff.d;  ${MAKE} -f troff.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

macros:	;  cd macros.d;  ${MAKE} -f macros.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

shells:	;  cd shells.d;  ${MAKE} -f shells.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

eqn:	;  cd eqn.d;  ${MAKE} -f eqn.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

neqn:	;  cd neqn.d; ${MAKE} -f neqn.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

tbl:	;  cd tbl.d;  ${MAKE} -f tbl.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

checkkmm: ;  cd checkmm; ${MAKE} -f checkmm.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

pic: ;  cd pic.d; ${MAKE} -f pic.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

old: ;  cd old.d; ${MAKE} -f old.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

macref: ;  cd macref.d; ${MAKE} -f macref.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

mmlint: ;  cd mmlint.d; ${MAKE} -f mmlint.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

sroff: ;  cd sroff.d; ${MAKE} -f sroff.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

x9700: ;  cd x9700.d; ${MAKE} -f x9700.mk ROOT=$(ROOT) CH=$(CH) ${ARGS}

install: ;  ${MAKE} -f text.mk ARGS=install ROOT=$(ROOT) CH=$(CH) ${ARGS}

clean:
	cd roff.d;  ${MAKE} -f roff.mk clean
	cd troff.d;  ${MAKE} -f troff.mk clean
	cd eqn.d;   ${MAKE} -f eqn.mk clean
	cd neqn.d;  ${MAKE} -f neqn.mk clean
	cd tbl.d;   ${MAKE} -f tbl.mk clean
	cd macros.d; ${MAKE} -f macros.mk clean
	cd shells.d; ${MAKE} -f shells.mk clean
	cd checkmm; ${MAKE} -f checkmm.mk clean
	cd pic.d; ${MAKE} -f pic.mk clean
	cd old.d; ${MAKE} -f old.mk clean
	cd macref.d; ${MAKE} -f macref.mk clean
	cd mmlint.d; ${MAKE} -f mmlint.mk clean
	cd sroff.d; ${MAKE} -f sroff.mk clean
	cd x9700.d; ${MAKE} -f x9700.mk clean

clobber:
	cd roff.d;  ${MAKE} -f roff.mk clobber
	cd troff.d;  ${MAKE} -f troff.mk clobber
	cd eqn.d;   ${MAKE} -f eqn.mk clobber
	cd neqn.d;  ${MAKE} -f neqn.mk clobber
	cd tbl.d;   ${MAKE} -f tbl.mk clobber
	cd macros.d;  ${MAKE} -f macros.mk clobber
	cd shells.d;  ${MAKE} -f shells.mk clobber
	cd checkmm; ${MAKE} -f checkmm.mk clobber
	cd pic.d; ${MAKE} -f pic.mk clobber
	cd old.d; ${MAKE} -f old.mk clobber
	cd macref.d; ${MAKE} -f macref.mk clobber
	cd mmlint.d; ${MAKE} -f mmlint.mk clobber
	cd sroff.d; ${MAKE} -f sroff.mk clobber
	cd x9700.d; ${MAKE} -f x9700.mk clobber

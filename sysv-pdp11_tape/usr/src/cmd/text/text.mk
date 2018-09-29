#	text sub-system make file
#	SCCS: @(#)text.mk	1.16

OL = /
SL = /usr/src/cmd
RDIR = ${SL}/text
ARGS = all
REL = current
MKSID = -r`gsid text.mk ${REL}`
LIST = lp
MAKE = make

compile all:	roff macros shells eqn neqn tbl checkkmm
	:

roff:	;  cd roff.d;  ${MAKE} -f roff.mk OL=${OL} ${ARGS} clean

macros:	;  cd macros.d;  ${MAKE} -f macros.mk OL=${OL} ${ARGS} clean

shells:	;  cd shells.d;  ${MAKE} -f shells.mk OL=${OL} ${ARGS} clean

eqn:	;  cd eqn.d;  ${MAKE} -f eqn.mk OL=${OL} ${ARGS} clean

neqn:	;  cd neqn.d; ${MAKE} -f neqn.mk OL=${OL} ${ARGS} clean

tbl:	;  cd tbl.d;  ${MAKE} -f tbl.mk OL=${OL} ${ARGS} clean

checkkmm: ;  cd checkmm; ${MAKE} -f checkmm.mk OL=${OL} ${ARGS} clean

install: ;  ${MAKE} -f text.mk ARGS=install OL=${OL} ${ARGS}

listing:	listmk
	cd roff.d;  ${MAKE} -f roff.mk LIST='${LIST}' listing
	cd eqn.d;   ${MAKE} -f eqn.mk LIST='${LIST}' listing
	cd neqn.d;  ${MAKE} -f neqn.mk LIST='${LIST}' listing
	cd tbl.d;   ${MAKE} -f tbl.mk LIST='${LIST}' listing
	cd macros.d; ${MAKE} -f macros.mk LIST='${LIST}' listing
	cd shells.d; ${MAKE} -f shells.mk LIST='${LIST}' listing
	cd checkmm;   ${MAKE} -f checkmm.mk LIST='${LIST}' listing
listmk: ;  pr text.mk | ${LIST}

build:	bldmk
	cd roff.d;  ${MAKE} -f roff.mk SL=${SL} REL=${REL} REWIRE='${REWIRE}' build
	cd eqn.d;   ${MAKE} -f eqn.mk SL=${SL} REL=${REL} REWIRE='${REWIRE}' build
	cd neqn.d;  ${MAKE} -f neqn.mk SL=${SL} REL=${REL} REWIRE='${REWIRE}' build
	cd tbl.d;   ${MAKE} -f tbl.mk SL=${SL} REL=${REL} REWIRE='${REWIRE}' build
	cd macros.d;  ${MAKE} -f macros.mk SL=${SL} REL=${REL} REWIRE='${REWIRE}' build
	cd shells.d;  ${MAKE} -f shells.mk SL=${SL} REL=${REL} REWIRE='${REWIRE}' build
	cd checkmm;  ${MAKE} -f checkmm.mk SL=${SL} REL=${REL} REWIRE='${REWIRE}' build
bldmk:  ;  get -p ${MKSID} s.text.mk > ${RDIR}/text.mk

mkedit:	;  get -e s.text.mk
mkdelta: ; delta s.text.mk

clean:
	cd roff.d;  ${MAKE} -f roff.mk clean
	cd eqn.d;   ${MAKE} -f eqn.mk clean
	cd neqn.d;  ${MAKE} -f neqn.mk clean
	cd tbl.d;   ${MAKE} -f tbl.mk clean
	cd macros.d; ${MAKE} -f macros.mk clean
	cd shells.d; ${MAKE} -f shells.mk clean
	cd checkmm; ${MAKE} -f checkmm.mk clean

clobber:
	cd roff.d;  ${MAKE} -f roff.mk clobber
	cd eqn.d;   ${MAKE} -f eqn.mk clobber
	cd neqn.d;  ${MAKE} -f neqn.mk clobber
	cd tbl.d;   ${MAKE} -f tbl.mk clobber
	cd macros.d;  ${MAKE} -f macros.mk clobber
	cd shells.d;  ${MAKE} -f shells.mk clobber
	cd checkmm; ${MAKE} -f checkmm.mk clobber

delete:
	cd roff.d;  ${MAKE} -f roff.mk delete
	cd eqn.d;   ${MAKE} -f eqn.mk delete
	cd neqn.d;  ${MAKE} -f neqn.mk delete
	cd tbl.d;   ${MAKE} -f tbl.mk delete
	cd macros.d;  ${MAKE} -f macros.mk delete
	cd shells.d;  ${MAKE} -f shells.mk delete
	cd checkmm;  ${MAKE} -f checkmm.mk delete

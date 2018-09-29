#	text Development Support Library (DSL) macros make file
#
# DSL 2
#	SCCS: @(#)macros.mk	1.44

OL = $(ROOT)/
LIST = lp
INS = :
INSMAC = ${OL}usr/lib/macros
INSTMAC = ${OL}usr/lib/tmac
INSLIB = ${OL}usr/lib
TMACFILES = tmac.an tmac.m tmac.org tmac.ptx tmac.osd tmac.v
MAKE = make
IFLAG = -n

compile all:	ntar mmn mmt vmca osdd man tmac ptx
	:
ntar:	ntar.c
	cc ${IFLAG} -O -s -o ntar ntar.c
mmn:	ntar macdir
	sh ./macrunch -n m ${MINS} -d ${OL}usr mmn

mmt:	ntar macdir
	sh ./macrunch -t m ${MINS} -d ${OL}usr mmt

vmca:	ntar macdir
	sh ./macrunch ${MINS} -d ${OL}usr vmca

org:	ntar macdir
	sh ./macrunch ${MINS} -d ${OL}usr org

osdd:	ntar macdir
	sh ./macrunch ${MINS} -d ${OL}usr osdd

man:	ntar macdir
	sh ./macrunch -n an ${MINS} -d ${OL}usr an
	sh ./macrunch -t an ${MINS} -d ${OL}usr an

ptx:	ntar macdir
	sh ./macrunch ${MINS} -d ${OL}usr ptx

macdir:
	-mkdir $(INSMAC)

tmac:
	-mkdir $(INSTMAC)
	$(INS) ${TMACFILES} ${INSTMAC}
	cd ${INSTMAC}; chmod 644 ${TMACFILES}; $(CH) chgrp bin $(TMACFILES); chown bin $(TMACFILES)

listing:    listmmn listmmt listvmca listorg listosdd listman listptx
	:
listmmn: ;  nl -ba mmn.src | pr -h "mmn.src" | ${LIST}
	    macref -s -t mmn.src | pr -h "macref of mmn.src" | ${LIST}
listmmt: ;  nl -ba mmt.src | pr -h "mmt.src" | ${LIST}
	    macref -s -t mmt.src | pr -h "macref of mmt.src" | ${LIST}
listvmca: ; nl -ba vmca.src | pr -h "vmca.src" | ${LIST}
	    macref -s -t vmca.src | pr -h "macref of vmca.src" | ${LIST}
listorg: ;  nl -ba org.src | pr -h "org.src" | ${LIST}
	    macref -s -t org.src | pr -h "macref of org.src" | ${LIST}
listosdd: ; nl -ba osdd.src | pr -h "osdd.src" | ${LIST}
	    macref -s -t osdd.src | pr -h "macref of osdd.src" | ${LIST}
listman: ;  nl -ba an.src | pr -h "an.src" | ${LIST}
	    macref -s -t an.src | pr -h "macref of an.src" | ${LIST}
listptx: ;  nl -ba ptx.src | pr -h "ptx.src" | ${LIST}
	    macref -s -t ptx.src | pr -h "macref of ptx.src" | ${LIST}

install:
	${MAKE} -f macros.mk MINS=-m INS=cp ROOT=$(ROOT) CH=$(CH)

insmmn:  ;  ${MAKE} -f macros.mk MINS=-m ROOT=$(ROOT) CH=$(CH) mmn
insmmt:  ;  ${MAKE} -f macros.mk MINS=-m ROOT=$(ROOT) CH=$(CH) mmt
insvmca: ;  ${MAKE} -f macros.mk MINS=-m ROOT=$(ROOT) CH=$(CH) vmca
insorg:  ;  ${MAKE} -f macros.mk MINS=-m ROOT=$(ROOT) CH=$(CH) org
insosdd: ;  ${MAKE} -f macros.mk MINS=-m ROOT=$(ROOT) CH=$(CH) osdd
insman:  ;  ${MAKE} -f macros.mk MINS=-m ROOT=$(ROOT) CH=$(CH) man
insptx:  ;  ${MAKE} -f macros.mk MINS=-m ROOT=$(ROOT) CH=$(CH) ptx
instmac: ;  ${MAKE} -f macros.mk INS=cp ROOT=$(ROOT) CH=$(CH) tmac

clean ntarclean mmnclean mmtclean vmcaclean orgclean osddclean ptxclean:
	:
clobber:  clean ntarclobber mmnclobber mmtclobber vmcaclobber \
		orgclobber osddclobber manclobber ptxclobber
	:
ntarclobber: ;  rm -f ntar
mmnclobber:  ;  rm -f mmn
mmtclobber:  ;  rm -f mmt
vmcaclobber: ;  rm -f vmca
orgclobber:  ;  rm -f org
osddclobber: ;  rm -f osdd
manclobber:  ;  rm -f an
ptxclobber:  ;  rm -f ptx

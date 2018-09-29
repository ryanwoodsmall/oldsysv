#	spell make file
#	SCCS:  @(#)spell.mk	1.2

OL = /
SL = /usr/src/cmd
RDIR = ${SL}/spell
INS = :
REL = current
CSID = -r`gsid spellcode ${REL}`
DSID = -r`gsid spelldata ${REL}`
SHSID = -r`gsid spell.sh ${REL}`
CMPRSID = -r`gsid compress.sh ${REL}`
MKSID = -r`gsid spell.mk ${REL}`
LIST = lp
CINSDIR = ${OL}usr/bin
PINSDIR = ${OL}usr/lib/spell
IFLAG = -i
SMFLAG =
CFLAGS = -O
SFILES = spellprog.c spellin.c
DFILES = american british local list extra stop
MAKE = make

compile all: spell hlista hlistb hstop spellin spellprog hashmake hashcheck
	:

spell:	spellprog spell.sh
	cp spell.sh spell
	$(INS) spell $(CINSDIR)
	chmod 755 ${CINSDIR}/spell
	@if [ "${OL}" = "/" ]; \
		then cd ${CINSDIR}; chown bin spell; chgrp bin spell; \
	   fi

spelldir:
	-mkdir ${PINSDIR}

compress:  spelldir compress.sh
	cp compress.sh compress
	${INS} compress ${PINSDIR}
	chmod 755 ${PINSDIR}/compress
	@if [ "${OL}" = "/" ]; \
		then cd ${PINSDIR}; chown bin compress; chgrp bin compress; \
	 fi

spellprog: spelldir spellprog.c hash.c hashlook.c huff.c malloc.c
	$(CC) $(IFLAG) ${SMFLAG} ${FFLAG} -s -O spellprog.c hash.c hashlook.c huff.c malloc.c -o spellprog
	$(INS) spellprog $(PINSDIR)
	chmod 755 ${PINSDIR}/spellprog
	@if [ "${OL}" = "/" ]; \
		then cd ${PINSDIR}; chown bin spellprog; chgrp bin spellprog; \
	 fi

spellin: spelldir spellin.c huff.c
	$(CC) $(IFLAG) ${SMFLAG} ${FFLAG} -O -s spellin.c huff.c -o spellin
	${INS} spellin ${PINSDIR}
	chmod 755 ${PINSDIR}/spellin
	@if [ "${OL}" = "/" ]; \
		then cd ${PINSDIR}; chown bin spellin; chgrp bin spellin; \
	 fi

hashcheck: spelldir hashcheck.c hash.c huff.c
	$(CC) $(IFLAG) ${SMFLAG} ${FFLAG} -O -s hashcheck.c hash.c huff.c -o hashcheck
	${INS} hashcheck ${PINSDIR}
	chmod 755 ${PINSDIR}/hashcheck
	@if [ "${OL}" = "/" ]; \
		then cd ${PINSDIR}; chown bin hashcheck; chgrp bin hashcheck; \
	fi

hashmake: spelldir hashmake.c hash.c
	$(CC) $(IFLAG) ${SMFLAG} ${FFLAG} -O -s hashmake.c hash.c -o hashmake
	${INS} hashmake ${PINSDIR}
	chmod 755 ${PINSDIR}/hashmake
	@if [ "${OL}" = "/" ]; \
		then cd ${PINSDIR}; chown bin hashmake; chgrp bin hashmake; \
	fi

alldata: hlista hlistb hstop
	rm htemp1

htemp1:	list local extra hashmake
	cat list local extra | ./hashmake >htemp1

hlista: spelldir american hashmake spellin htemp1
	./hashmake <american |sort -u - htemp1 >htemp2
	./spellin `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hlista
	$(INS) hlista $(PINSDIR)
	chmod 644 ${PINSDIR}/hlista
	@if [ "${OL}" = "/" ]; \
		then cd ${PINSDIR}; chown bin hlista; chgrp bin hlista; \
	fi
	rm htemp2

hlistb: spelldir british hashmake spellin htemp1
	./hashmake <british |sort -u - htemp1 >htemp2
	./spellin `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hlistb
	$(INS) hlistb $(PINSDIR)
	chmod 644 ${PINSDIR}/hlistb
	@if [ "${OL}" = "/" ]; \
		then cd ${PINSDIR}; chown bin hlistb; chgrp bin hlistb; \
	fi
	rm htemp2


hstop:	spelldir stop spellin hashmake
	./hashmake <stop | sort -u >htemp2
	./spellin `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hstop
	$(INS) hstop $(PINSDIR)
	chmod 644 ${PINSDIR}/hstop
	@if [ "${OL}" = "/" ]; \
		then cd ${PINSDIR}; chown bin hstop; chgrp bin hstop; \
	 fi
	rm htemp2

install:  ;  ${MAKE} -f spell.mk INS=cp OL=${OL} all
inssh:    ;  ${MAKE} -f spell.mk INS=cp OL=${OL} spell
inscomp:  ;  ${MAKE} -f spell.mk INS=cp OL=${OL} compress
inscode:  ;  ${MAKE} -f spell.mk INS=cp spell OL=${OL}
insdata:  ;  ${MAKE} -f spell.mk INS=cp alldata OL=${OL}

listing:  ;  pr spell.mk spell.sh compress.sh ${SFILES} ${DFILES} | ${LIST}
listmk:   ;  pr spell.mk | ${LIST}
listsh:	  ;  pr spell.sh | ${LIST}
listcomp: ;  pr compress.sh | ${LIST}
listcode: ;  pr ${SFILES} | ${LIST}
listdata: ;  pr ${DFILES} | ${LIST}

build:  bldmk bldsh bldcomp bldcode blddata
	:
bldcode:  ;  get -p ${CSID} s.spell.src ${REWIRE} | ntar -d ${RDIR} -g
blddata:  ;  get -p ${DSID} s.spell.data | ntar -d ${RDIR} -g
bldsh:	  ;  get -p ${SHSID} s.spell.sh ${REWIRE} > ${RDIR}/spell.sh
bldcomp:  ;  get -p ${CMPRSID} s.compress.sh ${REWIRE} > ${RDIR}/compress.sh
bldmk:    ;  get -p ${MKSID} s.spell.mk > ${RDIR}/spell.mk

edit:	sedit dedit mkedit shedit compedit
	:
sedit:	;  get -p -e s.spell.src | ntar -g
dedit:	;  get -p -e s.spell.data | ntar -g
shedit:	;  get -e s.spell.sh
compedit: ; get -e s.compress.sh

delta:	sdelta ddelta mkdelta shdelta compdelta
	:
sdelta:
	ntar -p ${SFILES} > spell.src
	delta s.spell.src
	rm -f ${SFILES}
ddelta:
	ntar -p ${DFILES} > spell.data
	delta s.spell.data
	rm -f ${DFILES}
shdelta:
	delta s.spell.sh
compdelta: ; delta s.compress.sh

mkedit:	;  get -e s.spell.mk
mkdelta: ; delta s.spell.mk

clean:
	rm -f *.o

clobber: clean shclobber compclobber
	rm -f spell spellprog spellin hlist* hstop
shclobber: ; rm -f spell
compclobber: ; rm -f compress

delete:	clobber shdelete compdelete
	rm -f ${SFILES} ${DFILES}
shdelete: shclobber
	rm -f spell.sh
compdelete: compclobber
	rm -f compress.sh

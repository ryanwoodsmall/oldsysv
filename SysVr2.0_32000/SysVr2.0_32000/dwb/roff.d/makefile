#	Development make file for nroff/otroff
#
# DSL 2.
#	SCCS:  @(#)makefile	1.25
#
# Regular make options:
#	nroff	make nroff
#	otroff	make otroff ('make PTAG= otroff')
#	suftab	make suftab
#	lint	lint the nroff/otroff source code
#	clean	remove temporary object files
#	clobber	remove all object files
#	delete	remove all source files
#	help	print this menu
#
# Development make options:
#	dnroff	make debug version of nroff (dnroff)
#	dotroff	make debug version of otroff (dotroff)
#	edit	prepare [nt]roff source for editing
#	delta	perform a delta on edited source
#	bldtab	generate name table builder program
#	listing	generate a listing (to bin ${BIN})


OL = $(ROOT)/
INS = :
CINSDIR = ${OL}usr/bin
DINSDIR = ${OL}usr/lib
STRIP = -s

PTAG = -DNROFF
IFLAG = -i
CFLAGS = -O $(PTAG) $(OPTS) ${FFLAG}
NTSOURCE = hytab.c n1.c n10.c n[2-9i].c nii.c ntab.c strs.hd t10.c \
	 t6.c tab3.c tdef.hd tw.hd uns.hd def.nr.names def.rq.names
SMSOURCE = suftabmake.c suftab.c
SOURCE = ${NTSOURCE} ${SMSOURCE}
NFILES = n1.o n2.o n3.o n4.o n5.o n6.o n7.o n8.o n9.o n10.o ni.o \
		nii.o ntab.o hytab.o
TFILES = n1.o n2.o n3.o n4.o n5.o t6.o n7.o n8.o n9.o t10.o ni.o \
		nii.o tab3.o hytab.o

nroff:	$(NFILES)
	${CC} $(IFLAG) $(OPTS) ${FFLAG} ${STRIP} -o nroff $(NFILES)

otroff:	checkPTAG $(TFILES)
	${CC} $(IFLAG) $(OPTS) ${FFLAG} ${STRIP} -o otroff $(TFILES)

checkPTAG:	;  @(if [ -n "$(PTAG)" ];\
			then (echo "must use PTAG="; exit 1);\
			else exit 0; fi)

n[18].o::	tdef.hd strs.hd tw.hd uns.hd
	:
n[2-79].o n10.o nii.o::	tdef.hd strs.hd tw.hd
	:
t6.o t10.o::	tdef.hd strs.hd
	:
ni.o::	tdef.hd nrtab.dt rqtab.dt
	:

#	***** NOTE *****  the 350 in the next line is hardwired for (NN)
nrtab.dt: bldtab def.nr.names
	./bldtab 350 def.nr.names > nrtab.dt

#	***** NOTE *****  the 400 in the next line is hardwired for (NM)
rqtab.dt:  bldtab def.rq.names
	./bldtab 400 def.rq.names > rqtab.dt

suftab:	suftabmake.c suftab.c
	cc suftabmake.c
	./a.out

bldtab:	bldtab.c tdef.hd
	cc -s -O -o bldtab bldtab.c

dnroff:	$(NFILES)
	${CC} $(IFLAG) $(OPTS) ${FFLAG} -o dnroff $(NFILES)
dotroff:	checkPTAG $(TFILES)
	${CC} $(IFLAG) $(OPTS) ${FFLAG} -o dotroff $(TFILES)

lint:	n1.c
	@lint -DNROFF -haxcs n[1-9].c n10.c ni.c nii.c ntab.c hytab.c |\
			pr -h "lint of nroff"
	@lint -haxcs n[1-5].c t6.c n[7-9].c t10.c ni.c nii.c tab3.c hytab.c |\
			pr -h "lint of otroff"

all install:
	@echo "wrong make file - use roff.mk";  exit 1

clean:
	rm -f *.o a.out

clobber:	clean bclobber
	rm -f nroff otroff suftab dnroff dotroff
bclobber:
	rm -f bldtab ??tab.dt

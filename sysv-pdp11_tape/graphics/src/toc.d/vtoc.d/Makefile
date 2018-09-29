# @(#)Makefile	1.2
I =	../../../include
BIN =	../../../bin
LIB =	../../../lib/glib.a
SOURCE = chcknds.c getentry.c getmax.c gpsfmt.c idcmp.c \
	levcal.c maxwordlen.c position.c setkids.c setwtst.c sizefac.c\
	squish.c treebldr.c upperstr.c validid.c vcmp.c vmap.c vtoc.c 
HDR =	$(I)/setop.h $(I)/util.h $(I)/errpr.h 

vtoc:	vtoc.a vtoc.h $(I)/gsl.h $(LIB)
	cc $(FFLAG) vtoc.a $(LIB) -lm -i -s -o $(BIN)/vtoc
	chmod 755 $(BIN)/vtoc
	touch vtoc

vtoc.a::	vtoc.c $(HDR)
		sh $(SHFLAG) ./cca vtoc.c
vtoc.a::	treebldr.c vtoc.h
		sh $(SHFLAG) ./cca treebldr.c
vtoc.a::	vmap.c vtoc.h
		sh $(SHFLAG) ./cca vmap.c
vtoc.a::	levcal.c vtoc.h
		sh $(SHFLAG) ./cca levcal.c
vtoc.a::	setkids.c vtoc.h
		sh $(SHFLAG) ./cca setkids.c
vtoc.a::	sizefac.c vtoc.h
		sh $(SHFLAG) ./cca sizefac.c
vtoc.a::	squish.c vtoc.h
		sh $(SHFLAG) ./cca squish.c
vtoc.a::	gpsfmt.c vtoc.h $(I)/gsl.h
		sh $(SHFLAG) ./cca gpsfmt.c
vtoc.a::	getentry.c vtoc.h
		sh $(SHFLAG) ./cca getentry.c
vtoc.a::	position.c vtoc.h
		sh $(SHFLAG) ./cca position.c
vtoc.a::	vcmp.c vtoc.h
		sh $(SHFLAG) ./cca vcmp.c
vtoc.a::	idcmp.c vtoc.h
		sh $(SHFLAG) ./cca idcmp.c
vtoc.a::	chcknds.c vtoc.h
		sh $(SHFLAG) ./cca chcknds.c
vtoc.a::	getmax.c vtoc.h
		sh $(SHFLAG) ./cca getmax.c
vtoc.a::	setwtst.c vtoc.h $(I)/gsl.h
		sh $(SHFLAG) ./cca setwtst.c
vtoc.a::	validid.c vtoc.h
		sh $(SHFLAG) ./cca validid.c
vtoc.a::	maxwordlen.c $(I)/util.h
		sh $(SHFLAG) ./cca maxwordlen.c
vtoc.a::	upperstr.c
		sh $(SHFLAG) ./cca upperstr.c

vtoc.h:	$(HDR); touch vtoc.h

listings: $(SOURCE) vtoc.h
	ls -l *.c *.h  | pr -h "  vtoc source files  "
	print $?
	touch listings

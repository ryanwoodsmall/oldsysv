# @(#)up.mk	1.1
all:	dzbstuff

dzbstuff:
	cd dzb; make -f dzb.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

clean:
	cd dzb; make -f dzb.mk clean

clobber:
	cd dzb; make -f dzb.mk clobber

FRC:

# @(#)pdp11.mk	1.1
all:	machine system drivers virtual pwbstuff micro

machine:
	cd ml; make -f ml.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

system:
	cd os; make -f os.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

drivers:
	cd io; make -f io.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

virtual:
	cd vt; make -f vt.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

pwbstuff:
	cd pwb; make -f pwb.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

micro:
	cd up; make -f up.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

clean:
	cd ml; make -f ml.mk clean
	cd os; make -f os.mk clean
	cd io; make -f io.mk clean
	cd vt; make -f vt.mk clean
	cd pwb; make -f pwb.mk clean
	cd up; make -f up.mk clean

clobber:
	cd ml; make -f ml.mk clobber
	cd os; make -f os.mk clobber
	cd io; make -f io.mk clobber
	cd vt; make -f vt.mk clobber
	cd pwb; make -f pwb.mk clobber
	cd up; make -f up.mk clobber

FRC:

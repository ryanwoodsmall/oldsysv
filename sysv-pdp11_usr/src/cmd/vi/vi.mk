# @(#) vi.mk 1.8@(#)
install: 
	sh makevi
#
# executed by makevi
#
all: cmd libterm termcap 

cmd:
	@echo "\n\t>Making commands."
	cd misc; make all; cd ..

libterm:
	@echo "\n\t>Making libtermlib.a"
	cd termlib; make all ; cd ..

termcap:
	@echo "\n\t>Ordering termcap & installing tabset."
	cd termcap.db ; make ; make install; make tabset

# make for Vax 11/780
src780:
	@echo "\n\t> Compiling ex source."
	cd vax ; make -f makefile.u3 all ; cd ..
	@echo "\n\t> Installing ex object."
	cd vax ; make install -f makefile.u3; cd ..

# make for Unix 370
src370:
	@echo "\n\t> Compiling ex source."
	cd vax ; make  -f makefile.370 all ; cd ..
	@echo "\n\t> Installing ex object."
	cd vax ; make install -f makefile.370; cd ..

# make for 3B-20S
src20S:
	@echo "\n\t> Compiling ex source."
	cd vax ; make  -f makefile.u3 all ; cd ..
	@echo "\n\t> Installing ex object."
	cd vax ; make install -f makefile.u3 ; cd ..

# make for Cory Pdp/11-70
srcc70:
	@echo "\n\t> Compiling ex source."
	cd vax ; make -f makefile.c70 all ; cd ..
	@echo "\n\t> Installing ex object."
	cd vax ; make install -f makefile.c70; cd ..

# make for Pdp/11-70
src70:
	@echo "\n\t> Compiling ex source."
	cd pdp11 ; make all -f makefile.u3 ; cd ..
	@echo "\n\t> Installing ex object."
	cd pdp11 ; make install -f makefile.u3 ; cd ..
	cd pdp11 ; make installutil -f makefile.u3; cd ..

manpages:
	@echo "\n\t> Installing man pages."
	cd doc/man ; make install -f man.mk

#
# Cleanup procedures
#
clobber:

clean:
	rm -f done.*
	cd misc ; make clean ; cd ..
	cd termlib ; make clean ; cd ..
	cd termcap.db ; make clean ; cd ..
	cd pdp11 ; make -f makefile.u3 clean ; cd ..

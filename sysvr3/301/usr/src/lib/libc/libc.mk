#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc:libc.mk	1.26"
#
# makefile for libc
#
#
# The variable PROF is null by default, causing both the standard C library
# and a profiled library to be maintained.  If profiled object is not 
# desired, the reassignment PROF=@# should appear in the make command line.
#
# The variable IGN may be set to -i by the assignment IGN=-i in order to
# allow a make to complete even if there are compile errors in individual
# modules.
#
# See also the comments in the lower-level machine-dependent makefiles.
#

VARIANT=
SGS=
CFLAGS=-O
PCFLAGS=
PFX=
CC=$(PFX)cc
AR=ar
LORDER=$(PFX)lorder
ROOT=
LIB=$(ROOT)/lib
LIBP=$(ROOT)/usr/lib/libp
DONE=
PROF=
NONPROF=
DEFLIST=

all:
	if pdp11 ; then make -e -f libc.mk specific MACHINE=pdp11 ; fi
	if vax ;   then make -e -f libc.mk specific MACHINE=vax ; fi
	if u3b ;   then make -e -f libc.mk specific MACHINE=u3b ; fi
	if u3b15 || u3b5 || u3b2 ;  then make -e -f libc.mk DEFLIST=-DM32 \
				specific MACHINE=m32 ; fi
	if u370 ;  then make -e -f libc.mk specific MACHINE=u370 ; fi

specific:
	#
	# compile portable library modules
	cd port; make -e CC=$(CC)
	#
	# compile machine-dependent library modules
	cd $(MACHINE); make -e CC=$(CC)
	#
	# place portable modules in "object" directory, then overlay
	# 	the machine-dependent modules.
	-rm -rf object
	mkdir object
	cp port/*/*.o object
	$(PROF)cp port/*/*.p object
	cp $(MACHINE)/*/*.o object
	$(PROF)cp $(MACHINE)/*/*.p object
	#
	if [ -d m32mau ]; then \
		cd m32mau; \
		make -e CC=$(CC); \
		cd .. ;\
		cp m32mau/*/*.o object; \
		$(PROF)cp m32mau/*/*.p object ;\
	fi
	#
	# delete temporary libraries
	-rm -f lib.libc
	$(PROF)-rm -f libp.libc
	#
	# set aside run-time modules, which don't go in library archive!
	cd object; for i in *crt?.o; do mv $$i ..; done
	#
	# build archive out of the remaining modules.
	cd object; make -e -f ../$(MACHINE)/makefile archive \
		AR=$(AR)  LORDER=$(LORDER) PROF=$(PROF)
	-rm -rf object
	#
	$(DONE)

move:
	#
	# move the library or libraries into the correct directory
	for i in *crt?.o; do cp $$i $(LIB)/$(SGS)$$i ; rm -f $$i ; done
	cp lib.libc $(LIB)/lib$(VARIANT)c.a ; rm -f lib.libc
	$(PROF)if [ ! -d $(LIBP) ]; then \
	$(PROF)	mkdir $(LIBP); \
	$(PROF)fi
	$(PROF)cp libp.libc $(LIBP)/lib$(VARIANT)c.a ; rm -f libp.libc

install: all move

clean:
	#
	# remove intermediate files except object modules and temp library
	-rm -rf lib*.contents obj*
	cd port ;  make clean
	if pdp11 ; then cd pdp11 ; make clean ; fi
	if vax ;   then cd vax ;   make clean ; fi
	if u3b ;   then cd u3b ;   make clean ; fi
	if u3b15 || u3b5 || u3b2 ;   then cd m32 ;   make clean ; fi
	if u370 ;  then cd u370 ;  make clean ; fi

clobber:
	#
	# remove intermediate files
	-rm -rf *.o lib*.libc lib*.contents obj*
	cd port ;  make clobber
	if pdp11 ; then cd pdp11 ; make clobber ; fi
	if vax ;   then cd vax ;   make clobber ; fi
	if u3b ;   then cd u3b ;   make clobber ; fi
	if u3b15 || u3b5 || u3b2;   then cd m32 ;   make clobber ; fi
	if [ -d m32mau ]; then \
		cd m32mau; \
		make clobber; \
		cd .. ;\
	fi
	if u370 ;  then cd u370 ;  make clobber ; fi

#	@(#)libc.mk	1.9
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
CFLAGS=-O
PCFLAGS=
PFX=
CC=$(PFX)cc
AR=ar
STRIP=$(PFX)strip
LORDER=$(PFX)lorder
ROOT=
LIB=$(ROOT)/lib
LIBP=$(ROOT)/lib/libp
PROF=
DONE=

all:
	if pdp11 ; then make -e -f libc.mk specific MACHINE=pdp11 ; fi
	if vax ;   then make -e -f libc.mk specific MACHINE=vax ; fi
	if ns32000  ;   then make -e -f libc.mk specific MACHINE=ns32000 ; fi
	if u3b ;   then make -e -f libc.mk specific MACHINE=u3b ; fi
	if u3b5 ;  then make -e -f libc.mk specific MACHINE=u3b5 ; fi
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
	cp port/*/*.p object
	cp $(MACHINE)/*/*.o object
	$(PROF)cp $(MACHINE)/*/*.p object
	#
	# delete temporary libraries
	-rm -f lib.libc
	$(PROF)-rm -f libp.libc
	#
	# set aside run-time modules, which don't go in library archive!
	cd object; for i in *crt0.o ; do mv $$i .. ; done
	#
	# build archive out of the remaining modules.
	cd object; make -e -f ../$(MACHINE)/makefile archive \
		AR=$(AR) STRIP=$(STRIP) LORDER=$(LORDER) PROF=$(PROF)
	-rm -rf object
	#
	$(DONE)

move:
	#
	# move the library or libraries into the correct directory
	for i in *crt0.o ; do cp $$i $(LIB)/$(VARIANT)$$i ; rm -f $$i ; done
	cp lib.libc $(LIB)/lib$(VARIANT)c.a ; rm -f lib.libc
	$(PROF)-mkdir $(LIBP)
	$(PROF)cp libp.libc $(LIBP)/lib$(VARIANT)c.a ; rm -f libp.libc

install: all move

clean:
	#
	# remove intermediate files except object modules and temp library
	-rm -rf lib*.contents obj*
	cd port ;  make clean
	if pdp11 ; then cd pdp11 ; make clean ; fi
	if vax ;   then cd vax ;   make clean ; fi
	if ns32000 ;   then cd ns32000 ;   make clean ; fi
	if u3b ;   then cd u3b ;   make clean ; fi
	if u3b5 ;   then cd u3b5 ;   make clean ; fi
	if u370 ;  then cd u370 ;  make clean ; fi

clobber:
	#
	# remove intermediate files
	-rm -rf *.o lib*.libc lib*.contents obj*
	cd port ;  make clobber
	if pdp11 ; then cd pdp11 ; make clobber ; fi
	if vax ;   then cd vax ;   make clobber ; fi
	if ns32000 ;   then cd ns32000 ;   make clobber ; fi
	if u3b ;   then cd u3b ;   make clobber ; fi
	if u3b5 ;   then cd u3b5 ;   make clobber ; fi
	if u370 ;  then cd u370 ;  make clobber ; fi

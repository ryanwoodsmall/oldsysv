#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lib:lib.mk	1.13"
#	Global Library Makefile
#
# SLIBLIST contains the list of library names for which a shared library can be 
# build
# To build a shared library only, type make -f lib.mk shared. The archive created
# will not be installed.
# To build a non-shared library only, type make -f lib.mk nonshared. The archive 
# created will not be installed.
# 
# 
LIB=$(ROOT)/lib
LIBP=$(ROOT)/lib/libp
USRLIB=$(ROOT)/usr/lib
LIBLIST=*
SLIBLIST=libc 
SGS=
LIBRARY=

all: nonshared shared
install: libs shlibs

nonshared:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; make -e -f $$i.mk; cd ..; \
	   fi \
	done
shared:
	for i in $(SLIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
		cd $$i; make -e -f sh$$i.mk; cd ../; \
	   fi \
	done


libs:
	for i in $(LIBLIST) ; \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; \
	   	make -e -f $$i.mk LIB=$(LIB) LIBP=$(LIBP) USRLIB=$(USRLIB) ULIBDIR=$(USRLIB) SGS=$(SGS) install; \
	   	cd ..; \
	   fi \
	done

shlibs:
	for i in $(SLIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
		cd $$i; \
		make -e -f sh$$i.mk LIB=$(LIB) LIBP=$(LIBP) USRLIB=$(USRLIB) ULIBDIR=$(USRLIB) SGS=$(SGS) install; \
		cd ..; \
	   fi \
	done
clean:
	for i in $(LIBLIST) $(SLIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; make -e -f $$i.mk clean; cd ..; \
	   fi \
	done

clobber:
	for i in $(LIBLIST) $(SLIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; make -e -f $$i.mk clobber; cd ..; \
	   fi \
	done

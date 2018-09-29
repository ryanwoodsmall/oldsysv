#ident	"@(#)lib:libmk.template	1.4"
#	Template for Global Library Makefile
#
#
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
LIBP=$(ROOT)/usr/lib/libp
SHLIB=$(ROOT)/shlib
LIBLIST=*
MAKE=make
SGS=

all:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; $(MAKE) -f $$i.mk; cd ..; \
	   fi \
	done

install:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; \
	   	$(MAKE) -f $$i.mk LIB=$(LIB) LIBP=$(LIBP) USRLIB=$(USRLIB) SGS=$(SGS) SHLIB=$(SHLIB) install; \
	   	cd ..; \
	   fi \
	done

clean:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; $(MAKE) -f $$i.mk clean; cd ..; \
	   fi \
	done

clobber:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; $(MAKE) -f $$i.mk clobber; cd ..; \
	   fi \
	done

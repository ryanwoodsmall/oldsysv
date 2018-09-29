#
#	@(#)lib.mk	6.2
#

LIBS=	cassi.a		\
		comobj.a	\
		mpwlib.a	\
		libPW.a

PRODUCTS = $(LIBS)

INSTR=

all: $(LIBS)
	@echo "SCCS Libraries are up to date."

cassi.a::
	cd cassi; $(MAKE) -e -f cassi.mk all

comobj.a::
	cd comobj; $(MAKE) -e -f comobj.mk all

mpwlib.a::
	cd mpwlib; $(MAKE) -e -f mpwlib.mk all

libPW.a::
	cd libPW; $(MAKE) -e -f libPW.mk all

install:	$(LIBS)
	cd cassi; $(MAKE) -e -f cassi.mk install
	cd comobj; $(MAKE) -e -f comobj.mk install
	cd mpwlib; $(MAKE) -e -f mpwlib.mk install
	cd libPW; $(MAKE) -e -f libPW.mk install

clean:
	cd cassi; $(MAKE) -e -f cassi.mk clean
	cd comobj; $(MAKE) -e -f comobj.mk clean
	cd mpwlib; $(MAKE) -e -f mpwlib.mk clean
	cd libPW; $(MAKE) -e -f libPW.mk clean

clobber:
	cd cassi; $(MAKE) -e -f cassi.mk clobber
	cd comobj; $(MAKE) -e -f comobj.mk clobber
	cd mpwlib; $(MAKE) -e -f mpwlib.mk clobber
	cd libPW; $(MAKE) -e -f libPW.mk clobber

.PRECIOUS:	$(PRODUCTS)

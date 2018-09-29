#ident	"@(#)sgs:sgs.mk.um32	1.4"
#	3b15/3b5/3b2 Cross-SGS Global Makefile
#	PATHEDIT MUST BE RUN BEFORE THIS MAKEFILE IS USED!
#
#

YACC=yacc
SGS=

all:	install libs
libs:
	cd ../../lib; make -f lib.mk install YACC="$(YACC)"

install:  sgs
	cd xenv/m32; make BINDIR=$(ROOT)/bin LIBDIR=$(ROOT)/lib install YACC="$(YACC)"

first:
	cd xenv/m32; make first YACC="$(YACC)"

sgs:	
	cd xenv/m32; $(MAKE) all YACC="$(YACC)"

uninstall:
	cd xenv/m32; make uninstall

save:
	cd xenv/m32; make save YACC="$(YACC)"

shrink:	clobber
	if [ true ] ; \
	then \
		cd ../../lib; make -f lib.mk clobber ; \
	fi

clobber:
	cd xenv/m32; make shrink

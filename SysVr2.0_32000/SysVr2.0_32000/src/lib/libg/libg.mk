#	@(#)libg.mk	1.4
all:;	@set +e;if vax ;then (cd vax; $(MAKE) -f libg.mk all); elif u3b; then (cd u3b; $(MAKE) -f libg.mk all); elif ns32000 ;then (cd ns32000; $(MAKE) -f libg.mk all);  fi
install:;	@set +e;if vax ;then (cd vax; $(MAKE) -f libg.mk install); elif u3b; then (cd u3b; $(MAKE) -f libg.mk install); elif ns32000 ;then (cd ns32000; $(MAKE) -f libg.mk install);  fi
clobber:;	@set +e;if vax ;then (cd vax; $(MAKE) -f libg.mk clobber); elif u3b; then (cd u3b; $(MAKE) -f libg.mk clobber); elif ns32000 ; then (cd ns32000; $(MAKE) -f libg.mk clobber); fi

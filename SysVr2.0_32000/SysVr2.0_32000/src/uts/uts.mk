# @(#)uts.mk	6.2
all:
	@-if vax ;\
	then cd vax;\
	make -f vax.mk "INS=$(INS)" "INSDIR=$(INSDIR)" "INCRT=$(INCRT)"\
		"FRC=$(FRC)" "SYS=$(SYS)" "NODE=$(NODE)" "REL=$(REL)"\
		"VER=$(VER)" "MACH=$(MACH)" "TYPE=$(TYPE)";\
	else if u3b;\
	then cd 3b;\
	make -f 3b.mk "INS=$(INS)" "INSDIR=$(INSDIR)" "INCRT=$(INCRT)"\
		"FRC=$(FRC)" "SYS=$(SYS)" "NODE=$(NODE)" "REL=$(REL)"\
		"VER=$(VER)" "MACH=$(MACH)" "TYPE=$(TYPE)";\
	else if ns32000;\
	then cd ns32000;\
	make -f ns.mk "INS=$(INS)" "INSDIR=$(INSDIR)" "INCRT=$(INCRT)"\
		"FRC=$(FRC)" "SYS=$(SYS)" "NODE=$(NODE)" "REL=$(REL)"\
		"VER=$(VER)" "MACH=$(MACH)" "TYPE=$(TYPE)";\
	else cd pdp11;\
	make -f pdp11.mk "INS=$(INS)" "INSDIR=$(INSDIR)" "INCRT=$(INCRT)"\
		"FRC=$(FRC)" "SYS=$(SYS)" "NODE=$(NODE)" "REL=$(REL)"\
		"VER=$(VER)" "MACH=$(MACH)" "TYPE=$(TYPE)";\
	fi;fi;fi

clean:
	@-if vax ; then cd vax; make -f vax.mk clean ;\
	else if u3b; then cd 3b; make -f 3b.mk clean ;\
	else if ns32000; then cd ns32000; make -f ns.mk clean ;\
	else cd pdp11; make -f pdp11.mk clean ; fi; fi; fi

clobber:
	@-if vax ; then cd vax; make -f vax.mk clobber "SYS=$(SYS)" "VER=$(VER)" ; \
	else if u3b; then cd 3b; make -f 3b.mk clobber "SYS=$(SYS)" "VER=$(VER)"; \
	else if ns32000; then cd ns32000; make -f ns.mk clobber "SYS=$(SYS)" "VER=$(VER)"; \
	else cd pdp11; make -f pdp11.mk clobber "SYS=$(SYS)" "VER=$(VER)" ; fi; fi; fi

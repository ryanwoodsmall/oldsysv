# @(#)uts.mk	1.1
all:
	@-if /bin/vax ;\
	then cd vax;\
	make -f vax.mk "INS=$(INS)" "INSDIR=$(INSDIR)" "INCRT=$(INCRT)"\
		"FRC=$(FRC)" "SYS=$(SYS)" "NODE=$(NODE)" "REL=$(REL)"\
		"VER=$(VER)" "MACH=$(MACH)" "TYPE=$(TYPE)";\
	else cd pdp11;\
	make -f pdp11.mk "INS=$(INS)" "INSDIR=$(INSDIR)" "INCRT=$(INCRT)"\
		"FRC=$(FRC)" "SYS=$(SYS)" "NODE=$(NODE)" "REL=$(REL)"\
		"VER=$(VER)" "MACH=$(MACH)" "TYPE=$(TYPE)";\
	fi

clean:
	@-if /bin/vax ; then cd vax; make -f vax.mk clean ;\
	else cd pdp11; make -f pdp11.mk clean ; fi

clobber:
	@-if /bin/vax ; then cd vax; make -f vax.mk clobber "SYS=$(SYS)" "VER=$(VER)" ; \
	else cd pdp11; make -f pdp11.mk clobber "SYS=$(SYS)" "VER=$(VER)" ; fi

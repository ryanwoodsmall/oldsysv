#	@(#)nm.mk	2.2

#	nm.mk nm makefile

INSDIR = /bin
CURDIR = ..

nm:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) INSDIR=$(CURDIR); \
	else \
		@echo 'Cannot make nm command: unknown target procesor.'; \
	fi



clean:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) clean; \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) clean; \
	fi


install: nm
	cp nm $(INSDIR)/nm
	-rm -f nm

clobber: clean
	-rm -f nm

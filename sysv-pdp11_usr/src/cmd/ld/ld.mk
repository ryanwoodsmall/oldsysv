#	@(#)ld.mk	2.3

#	ld.mk ld makefile

INSDIR = /bin
CURDIR = ..

ld:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) INSDIR=$(CURDIR); \
	else \
		@echo 'Cannot make ld command: unknown target procesor.'; \
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


install:  ld
	cp ld $(INSDIR)/ld
	-rm -f ld

clobber: clean
	-rm -f ld

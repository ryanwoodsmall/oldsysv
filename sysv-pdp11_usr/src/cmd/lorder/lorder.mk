#	@(#)lorder.mk	2.2

#	lorder.mk lorder makefile

INSDIR = /bin
CURDIR = ..

lorder:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) INSDIR=$(CURDIR); \
	else \
		@echo 'Cannot make lorder command: unknown target procesor.'; \
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


install: lorder
	cp lorder $(INSDIR)/lorder
	-rm -f lorder

clobber: clean
	-rm -f lorder

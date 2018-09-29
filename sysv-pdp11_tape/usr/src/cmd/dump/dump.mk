#	@(#)dump.mk	2.4

#	dump.mk dump makefile

INSDIR = /bin
CURDIR = ..

dump:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) INSDIR=$(CURDIR); \
	else \
		@echo 'Cannot make dump command: unknown target procesor.'; \
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


install:  dump
	-if vax; \
	then \
		cp dump $(INSDIR)/dump; \
		rm -f dump; \
	fi

clobber: clean
	-rm -f dump

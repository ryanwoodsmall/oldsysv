#	@(#)strip.mk	2.2

#	strip.mk strip makefile

INSDIR = /bin
CURDIR = ..

strip:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) INSDIR=$(CURDIR); \
	else \
		@echo 'Cannot make strip command: unknown target procesor.'; \
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


install: strip
	cp strip $(INSDIR)/strip
	-rm -f strip

clobber: clean
	-rm -f strip

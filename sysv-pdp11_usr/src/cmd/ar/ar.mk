#	@(#)ar.mk	2.4

#	ar.mk ar makefile

INSDIR = /bin
CURDIR = ..

ar:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) INSDIR=$(CURDIR); \
	else \
		echo 'Cannot make ar command: unknown target procesor.'; \
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
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) clean; \
	fi


install: ar
	cp ar $(INSDIR)/ar

clobber: clean
	-rm -f ar

#	@(#) as.mk: 2.9 12/11/82

#	as.mk as makefile

ROOT =
BINDIR = $(ROOT)/bin
#
# CFLAGS should also be passed down, but that won't be done until
# the makefiles are standardized
#

as all:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" ; \
		cd ../lj_vax; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" ; \
	elif pdp11; \
	then \
		cd pdp; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" ; \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" ; \
	elif ns32000; \
	then \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" ; \
	else \
		echo 'Cannot make as command: unknown target procesor.'; \
	fi



clean:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) clean; \
		cd ../lj_vax; \
		$(MAKE) clean; \
	elif pdp11; \
	then \
		cd pdp; \
		$(MAKE) clean; \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) clean; \
	elif ns32000; \
	then \
		cd ns32000; \
		$(MAKE) clean; \
	fi


install: 
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
		cd ../lj_vax; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	elif pdp11; \
	then \
		cd pdp; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	elif ns32000; \
	then \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	fi

clobber: 
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) clobber; \
		cd ../lj_vax; $(MAKE) -$(MAKEFLAGS) clobber; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) clobber; \
	elif pdp11; then \
		cd pdp; $(MAKE) -$(MAKEFLAGS) clobber; \
	elif ns32000; then \
		cd ns32000; $(MAKE) -$(MAKEFLAGS) clobber; \
	fi

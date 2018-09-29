#	@(#) ld.mk: 2.6 12/11/82

#	ld.mk ld makefile

ROOT =
BINDIR = $(ROOT)/bin
#
# CFLAGS should also be passed down, but this won't be done until
# all the makefiles have been standardized
#

all ld:
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	elif ns32000; then \
		cd ns32000; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	elif pdp11; then \
		cd pdp; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	else \
		echo 'Cannot make ld command: unknown target procesor.'; \
	fi



clean:
	-if vax; then \
		cd vax; $(MAKE) clean; \
	elif ns32000; then \
		cd ns32000; $(MAKE) clean; \
	elif pdp11; then \
		cd pdp; $(MAKE) clean; \
	elif u3b; then \
		cd u3b; $(MAKE) clean; \
	fi


install: 
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	elif ns32000; then \
		cd ns32000; $(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	elif pdp11; then \
		cd pdp; $(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	fi

clobber: 
	-if vax; then \
		cd vax; $(MAKE) clobber; \
	elif ns32000; then \
		cd ns32000; $(MAKE) clobber; \
	elif u3b; then \
		cd u3b; $(MAKE) clobber; \
	elif pdp11; then \
		cd pdp; $(MAKE) clobber; \
	fi

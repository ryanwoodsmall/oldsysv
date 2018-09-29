#	@(#) size.mk 2.5 12/11/82

#	size.mk size makefile

ROOT	=
BINDIR	= $(ROOT)/bin
#
# CFLAGS should also be passed down, but this won't be done until
# all the makefiles have been standardized
#

all size:
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	elif ns32000; then \
		cd ns32000; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	elif pdp11; then \
		cd pdp; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	else \
		echo 'Cannot make size command: unknown target procesor.'; \
	fi



clean:
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) clean; \
	elif ns32000; then \
		cd ns32000; $(MAKE) -$(MAKEFLAGS) clean; \
	elif pdp11; then \
		cd pdp; $(MAKE) -$(MAKEFLAGS) clean; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) clean; \
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
		cd vax; $(MAKE) -$(MAKEFLAGS) clobber; \
	elif ns32000; then \
		cd ns32000; $(MAKE) -$(MAKEFLAGS) clobber; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) clobber; \
	elif pdp11; then \
		cd pdp; $(MAKE) -$(MAKEFLAGS) clobber; \
	fi

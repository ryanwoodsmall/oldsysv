#	@(#) lorder.mk 2.5 12/11/82

#	lorder.mk lorder makefile

ROOT	=
BINDIR	= $(ROOT)/bin

all lorder:
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS); \
	elif ns32000; then \
		cd ns32000; $(MAKE) -$(MAKEFLAGS); \
	elif pdp11; then \
		cd pdp; $(MAKE) -$(MAKEFLAGS); \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS); \
	else \
		echo 'Cannot make lorder command: unknown target procesor.'; \
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
		cd vax; $(MAKE) -$(MAKEFLAGS) BINDIR=$(BINDIR) install; \
	elif ns32000; then \
		cd ns32000; $(MAKE) -$(MAKEFLAGS) BINDIR=$(BINDIR) install; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) BINDIR=$(BINDIR) install; \
	elif pdp11; then \
		cd pdp; $(MAKE) -$(MAKEFLAGS) BINDIR=$(BINDIR) install; \
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

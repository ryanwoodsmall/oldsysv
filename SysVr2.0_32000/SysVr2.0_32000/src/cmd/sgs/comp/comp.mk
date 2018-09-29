#	@(#) comp.mk: 1.2 12/13/82

#	comp.mk comp makefile

ROOT =
BINDIR = $(ROOT)/bin
#
# This should pass CFLAGS down, but can't until the lower level
# makefile is standardized
#
all comp:
	-if u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	else \
		echo 'Cannot make comp command: unknown target procesor.'; \
	fi



clean:
	-if u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) clean; \
	fi


install: 
	-if u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	fi

clobber: 
	-if u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) clobber; \
	fi

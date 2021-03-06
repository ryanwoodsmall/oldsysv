#	@(#) list.mk: 1.1 12/10/82

#	list.mk list makefile

ROOT =
BINDIR = $(ROOT)/bin
#
# CFLAGS should also be passed down, but this won't be done until
# all the makefiles have been standardized
#

all list:
	-if u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	else \
		echo 'Cannot make list command: unknown target procesor.'; \
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

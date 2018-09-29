#	@(#) libld.mk: 1.1 12/10/82

#	libld.mk libld makefile

ROOT =
LIBDIR = $(ROOT)/lib
#
# This makefile should also pass down CFLAGS, but this won't be
# done until all the lower level makefiles have been standardized
#

all libld:
	-if u3b || vax || ns32000; then \
		cd common; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	else \
		echo 'Cannot make libld library: unknown target procesor.'; \
	fi



clean:
	-if u3b || vax || ns32000; then \
		cd common; $(MAKE) -$(MAKEFLAGS) clean; \
	fi


install: 
	-if u3b || vax || ns32000; then \
		cd common; $(MAKE) -$(MAKEFLAGS) CC="$(CC)" LIBDIR=$(LIBDIR) install; \
	fi

clobber: 
	-if u3b || vax || ns32000; then \
		cd common; $(MAKE) -$(MAKEFLAGS) clobber; \
	fi

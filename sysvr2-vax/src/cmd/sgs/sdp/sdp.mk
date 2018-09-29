#	@(#) sdp.mk: 1.2 12/15/82

#	sdp.mk sdp makefile

ROOT =
BINDIR = $(ROOT)/bin
#
# CFLAGS should also be passed down, but this won't be done until
# all the makefiles have been standardized
#

all sdp:
	-if u3b; then \
		cd common; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	else \
		echo 'Cannot make sdp library: unknown target procesor.'; \
	fi



clean:
	-if u3b; then \
		cd common; $(MAKE) -$(MAKEFLAGS) clean; \
	fi


install: all 

clobber: 
	-if u3b; then \
		cd common; $(MAKE) -$(MAKEFLAGS) clobber; \
	fi

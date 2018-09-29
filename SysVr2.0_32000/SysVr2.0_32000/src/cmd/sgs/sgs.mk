#	@(#) sgs.mk: 1.3 12/24/82
#
#	SGS Global Makefile
#
ROOT	=
BINDIR	= $(ROOT)/bin
LIBDIR	= $(ROOT)/lib
SGS	=
#
TOOLS	= libld as cmd optim comp ld sdp compress dis dump list lorder nm \
		size strip ar unix_conv
#
all install clean clobber:
	cd as; \
	for tool in $(TOOLS); \
	do \
		echo "Making $$tool $@"; \
		cd ../$$tool; \
		$(MAKE) -$(MAKEFLAGS) SGS=$(SGS) CC="$(CC)" \
		 	BINDIR=$(BINDIR) LIBDIR=$(LIBDIR) \
			-f $$tool.mk $@; \
	done
#
sgs:	all
#
# This makefile should also pass down CFLAGS, but this will
# not be done until the makefiles have all been standardized
#

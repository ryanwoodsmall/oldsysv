#	@(#)unix_conv.mk	2.5 12/12/82

#	convert.mk convert makefile

ROOT	=
BINDIR	= $(ROOT)/bin

all convert:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)"; \
	elif ns32000; \
	then \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)"; \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)"; \
	else \
		echo 'Cannot make convert command: unknown target procesor.'; \
	fi



clean:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS)  clean; \
	elif ns32000; \
	then \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS)  clean; \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) -$(MAKEFLAGS)  clean; \
	fi


install:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)" \
			BINDIR=$(BINDIR) install; \
	elif ns32000; \
	then \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)" \
			BINDIR=$(BINDIR) install; \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)" \
			BINDIR=$(BINDIR) install; \
	fi

clobber: 
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) clobber; \
	elif ns32000; \
	then \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) clobber; \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) -$(MAKEFLAGS) clobber; \
	fi

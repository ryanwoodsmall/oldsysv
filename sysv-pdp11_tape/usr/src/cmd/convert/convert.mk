#	@(#)convert.mk	2.3

#	convert.mk convert makefile

INSDIR = /bin
CURDIR = ..

convert:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) INSDIR=$(CURDIR); \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) INSDIR=$(CURDIR); \
	else \
		@echo 'Cannot make convert command: unknown target procesor.'; \
	fi



clean:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) clean; \
	elif u3b; \
	then \
		cd u3b; \
		$(MAKE) clean; \
	fi


install:  convert
	-if vax; \
	then \
		cp convert $(INSDIR)/convert; \
		rm -f convert; \
	elif u3b; \
	then \
		cp convert $(INSDIR)/convert; \
		rm -f convert; \
	fi

clobber: clean
	-rm -f convert

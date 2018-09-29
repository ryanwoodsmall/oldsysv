# @(#)profiler.mk	1.3
FFLAG =

CC = cc

CFLAGS = -O

install:
	-if ns32000; then cd ns32000; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; \
		fi
	-if vax; then cd vax; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; \
		fi
	-if pdp11; then cd pdp11; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; fi
	-if u3b; then cd u3b20; \
		make -f profiler.mk ARGS="$(ARGS)" CFLAGS="$(CFLAGS)" \
		FFLAG="$(FFLAG)" install clobber; fi

clobber:

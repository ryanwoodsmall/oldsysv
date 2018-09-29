#	kmcrje.mk 1.5 of 9/30/81
#	@(#)kmcrje.mk	1.5

rje1:	send
	cd src; make -f src.mk install
	cd util; make -f util.mk install
	@if u3b || vax || pdp11; then \
	cd vpm; make -f vpm.mk install; \
	fi
	cd src; make -f rje1.mk

rje2:	rje1
	cd src; make -f rje2.mk

rje3:	rje2
	cd src; make -f rje3.mk

rje4:	rje3
	cd src; make -f rje4.mk

send:
	if u370; then \
	cd send.d; make CCFLAGS="-O -b1,0" -f send.mk install; \
	else \
	cd send.d; make -f send.mk install; \
	fi

rjestat:
	cd util; make -f util.mk rjestat

install:
	make -f rje.mk $(ARGS)

clobber:
	cd src; make -f src.mk clean
	cd send.d; make -f send.mk clean
	cd util; make -f util.mk clean
	@if u3b || vax || pdp11; then \
	cd vpm; make -f vpm.mk clean; \
	fi
	cd lib; make -f lib.mk clean

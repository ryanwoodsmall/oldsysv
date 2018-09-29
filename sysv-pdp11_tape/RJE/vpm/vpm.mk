#	vpm.mk 1.1 of 9/30/81
#	@(#)vpm.mk	1.1
#
VPMC = vpmc

ARGS = -m

install:	rjeproto

rjeproto:	rjeproto.c rjeproto.r
	@if u3b; then \
	$(VPMC) -o rjeproto rjeproto.c; \
	else \
	$(VPMC) $(ARGS) -o rjeproto rjeproto.r; \
	fi
	cp rjeproto /lib/rjeproto
	-chmod 644 /lib/rjeproto
	-chown rje /lib/rjeproto
	-chgrp bin /lib/rjeproto

clean:
	-rm -f rjeproto

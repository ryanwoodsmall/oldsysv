#
#	@(#)prof.mk	1.2
#
ROOT	=
VERSION	=
UBINDIR	= $(ROOT)/usr/bin
USRINC	= $(ROOT)/usr/$(VERSION)/include

all:	prof.o
	if pdp11; then \
		$(CC) $(CFLAGS) prof.o -o prof; \
	else \
		$(CC) $(CFLAGS) prof.o -lld -o prof; \
	fi

prof:	all $(USRINC)/stdio.h $(USRINC)/a.out.h $(USRINC)/sys/types.h \
	$(USRINC)/sys/param.h $(USRINC)/mon.h

install:	prof
	cp prof $(UBINDIR)/prof
	chmod 775 $(UBINDIR)/prof
	chgrp bin $(UBINDIR)/prof
	chown bin $(UBINDIR)/prof

clean:
	rm -f prof.o

clobber: clean
	rm -f prof

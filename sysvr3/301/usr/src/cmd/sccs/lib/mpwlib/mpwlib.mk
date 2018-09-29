#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:lib/mpwlib/mpwlib.mk	1.7"
#
#
ROOT =

AR = ar
LORDER = lorder

CFLAGS = -O

LIB = ../mpwlib.a

PRODUCTS = $(LIB)

FILES = fmalloc.o		\
		xcreat.o	\
		xmsg.o

all: $(PRODUCTS)
	@echo "Library $(PRODUCTS) is up to date\n"

$(LIB): $(FILES)
	$(AR) cr $(LIB) `$(LORDER) *.o | tsort`
	$(CH) chmod 664 $(LIB)

xcreat: ../../hdr/defines.h
xmsg:	../../hdr/defines.h

install: $(LIB)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(PRODUCTS)

.PRECIOUS:	$(PRODUCTS)

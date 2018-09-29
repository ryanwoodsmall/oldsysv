#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:lib/cassi/cassi.mk	1.7"
#
#
ROOT =

AR = ar
LORDER = lorder

CFLAGS = -O

LIB = ../cassi.a

PRODUCTS = $(LIB)

FILES = gf.o		\
		cmrcheck.o	\
		deltack.o	\
		error.o		\
		filehand.o

all: $(PRODUCTS)
	@echo "Library $(PRODUCTS) is up to date\n"

$(LIB): $(FILES)
	$(AR) cr $(LIB) `$(LORDER) *.o | tsort`
	$(CH) chmod 664 $(LIB)

gf.o:	gf.c	\
	 ../../hdr/filehand.h

cmrcheck.o:	cmrcheck.c	\
	 ../../hdr/filehand.h

deltack.o:	deltack.c	\
	 ../../hdr/filehand.h	\
	 ../../hdr/had.h	\
	 ../../hdr/defines.h

filehand.o:	filehand.c ../../hdr/filehand.h

install:	$(PRODUCTS)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(PRODUCTS)


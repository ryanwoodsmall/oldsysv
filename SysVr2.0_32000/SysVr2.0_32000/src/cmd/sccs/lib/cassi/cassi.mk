#
#	@(#)cassi.mk	1.5
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
	chmod 664 $(LIB)

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


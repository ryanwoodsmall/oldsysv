#
#	@(#)mpwlib.mk	1.5
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
	chmod 664 $(LIB)

xcreat: ../../hdr/defines.h
xmsg:	../../hdr/defines.h

install: $(LIB)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(PRODUCTS)

.PRECIOUS:	$(PRODUCTS)

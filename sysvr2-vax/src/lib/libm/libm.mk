#	@(#)libm.mk	1.8
#
# makefile for libm
#
# Note that the object list given in FILES is in the order in which the files
# should be added to the archive when it is created.  If the files are not
# ordered correctly, linking of libm fails on the pdp11 and is slowed down
# on other machines.

INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
LIBP=$(ROOT)/lib/libp
VARIANT =
LIBNAME = lib$(VARIANT)m.a
AR=ar
FILES =\
	$(LIBNAME)(asin.o)\
	$(LIBNAME)(atan.o)\
	$(LIBNAME)(erf.o)\
	$(LIBNAME)(fabs.o)\
	$(LIBNAME)(floor.o)\
	$(LIBNAME)(fmod.o)\
	$(LIBNAME)(gamma.o)\
	$(LIBNAME)(hypot.o)\
	$(LIBNAME)(jn.o)\
	$(LIBNAME)(j0.o)\
	$(LIBNAME)(j1.o)\
	$(LIBNAME)(pow.o)\
	$(LIBNAME)(log.o)\
	$(LIBNAME)(sin.o)\
	$(LIBNAME)(sinh.o)\
	$(LIBNAME)(sqrt.o)\
	$(LIBNAME)(tan.o)\
	$(LIBNAME)(tanh.o)\
	$(LIBNAME)(exp.o)\
	$(LIBNAME)(matherr.o)

all: $(LIBNAME) profiledlib

$(LIBNAME): $(FILES)
	$(CC) -c $(CFLAGS) $(?:.o=.c)
	$(AR) rv $(LIBNAME) $?
	rm $?

$(FILES): $(INC)/math.h $(INC)/values.h $(INC)/errno.h \
		$(INC)/sys/errno.h

.c.a:;

profiledlib:
	make -f libm.mk -e libp.$(LIBNAME) LIBNAME=libp.$(LIBNAME) \
						CFLAGS="-p $(CFLAGS)"

install: all
	cp $(LIBNAME) $(LIB)/$(LIBNAME)
	cp libp.$(LIBNAME) $(LIBP)/$(LIBNAME)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME) libp.$(LIBNAME)

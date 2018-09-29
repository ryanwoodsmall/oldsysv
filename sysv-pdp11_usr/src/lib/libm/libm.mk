#	@(#)libm.mk	1.5
#
# makefile for libm
#
# Note that the object list given in FILES is in the order in which the files
# should be added to the archive when it is created.  If the files are not
# ordered correctly, linking of libm fails on the pdp11 and is slowed down
# on other machines.

LIB=/lib
LIBP=/lib/libp
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
	$(LIBNAME)(sqrt.o)\
	$(LIBNAME)(tan.o)\
	$(LIBNAME)(tanh.o)\
	$(LIBNAME)(sinh.o)\
	$(LIBNAME)(exp.o)\
	$(LIBNAME)(matherr.o)

all: $(LIBNAME) profiledlib

$(LIBNAME): $(FILES)
	$(CC) -c $(CFLAGS) $(?:.o=.c)
	$(AR) rv $(LIBNAME) $?
	rm $?

$(FILES): /usr/include/math.h /usr/include/values.h /usr/include/errno.h \
		/usr/include/sys/errno.h

.c.a:;

profiledlib:
	make -f libm.mk -e libp.$(LIBNAME) LIBNAME=libp.$(LIBNAME) \
						CFLAGS="-p $(CFLAGS)"

mq: mq.o $(LIBNAME)
	$(CC) $(CFLAGS) mq.o $(LIBNAME) -o mq

mq.o: asin.test.h atan.test.h constants.h exp.test.h gamma.test.h \
	log.test.h machar.h pow.test.h prnt.env.h sin.test.h sinh.test.h \
	sqrt.test.h tan.test.h tanh.test.h tests.h \

	$(CC) $(CFLAGS) -c mq.c

install: all
	mv $(LIBNAME) $(LIB)/$(LIBNAME)
	mv libp.$(LIBNAME) $(LIBP)/$(LIBNAME)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME) libp.$(LIBNAME)

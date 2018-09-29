#
#	@(#)comobj.mk	1.6
#
ROOT =

AR = ar
LORDER = lorder

CFLAGS = -O

LIB = ../comobj.a

PRODUCTS = $(LIB)

FILES = auxf.o		\
		chkid.o		\
		chksid.o	\
		date_ab.o	\
		date_ba.o	\
		del_ab.o	\
		del_ba.o	\
		dodelt.o	\
		dofile.o	\
		dohist.o	\
		doie.o		\
		dolist.o	\
		eqsid.o		\
		flushto.o	\
		fmterr.o	\
		getline.o	\
		logname.o	\
		newstats.o	\
		permiss.o	\
		pf_ab.o		\
		putline.o	\
		rdmod.o		\
		setup.o		\
		sid_ab.o	\
		sid_ba.o	\
		sidtoser.o	\
		sinit.o		\
		stats_ab.o

all: $(PRODUCTS)
	@echo "Library $(PRODUCTS) is up to date\n"

$(LIB): $(FILES)
	$(AR) cr $(LIB) `$(LORDER) *.o | tsort`
	chmod 664 $(LIB)

auxf.o: ../../hdr/defines.h
chkid.o: ../../hdr/defines.h
chksid.o: ../../hdr/defines.h
date_ab.o:
date_ba.o: ../../hdr/defines.h
del_ab.o: ../../hdr/defines.h
del_ba.o: ../../hdr/defines.h
dodelt.o: ../../hdr/defines.h
dofile.o: ../../hdr/defines.h
dohist.o: ../../hdr/defines.h ../../hdr/had.h
doie.o: ../../hdr/defines.h
dolist.o: ../../hdr/defines.h
eqsid.o: ../../hdr/defines.h
flushto.o: ../../hdr/defines.h
fmterr.o: ../../hdr/defines.h
getline.o: ../../hdr/defines.h
logname.o:
newstats.o: ../../hdr/defines.h
permiss.o: ../../hdr/defines.h
pf_ab.o: ../../hdr/defines.h
putline.o: ../../hdr/defines.h
rdmod.o: ../../hdr/defines.h
setup.o: ../../hdr/defines.h
sid_ab.o: ../../hdr/defines.h
sid_ba.o: ../../hdr/defines.h
sidtoser.o: ../../hdr/defines.h
sinit.o: ../../hdr/defines.h
stats_ab.o: ../../hdr/defines.h

install:	$(LIB)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(PRODUCTS)

.PRECIOUS:	$(PRODUCTS)

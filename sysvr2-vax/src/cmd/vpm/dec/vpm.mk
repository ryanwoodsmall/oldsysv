#	vpm.mk 1.6.1.1 of 7/20/84
#	@(#)vpm.mk	1.6.1.1
#
FRC =
INS = /etc/install -n /usr/bin
INSE = /etc/install -n /etc
INSL = /etc/install -n /lib
INSV = /etc/install -f /usr/lib/vpm
CFLAGS = -O -s


all:	vpmc vpmstart vpmset dmkset vpmsave vpmfmt vpmtest vpmfsr

vpmc:	pl mpl pss0 pss12 plistg vratfor

pl:
	cd plsrc;\
	make -f pl.mk all;\
	mv pl ..

mpl:
	cd mplsrc;\
	make -f mpl.mk all;\
	mv mpl ..

vratfor:
	cd ratsrc;\
	make -f ratfor.mk all;\
	mv ratfor ../vratfor

pss0:	pass0.o optfns.o
	$(CC) -o pss0 pass0.c optfns.o

pass0.o: pass0.h pass0.c
	$(CC) -c pass0.c

optfns.o: pass0.h optfns.c
	$(CC) -c optfns.c

plistg:
	cc -O -o plistg plistg.c

pss12:
	cc -O -o pss12 pass12.c

vpmstart:

vpmset:

dmkset:

vpmsave:

vpmfmt:

vpmtest:

vpmfsr:

install:    all
	   $(INSV) pl
	   $(INSV) mpl
	   $(INSV) vratfor
	   $(INSV) pss0
	   $(INSV) pss12
	   $(INSV) plistg
	    -mkdir /usr/lib/vpm/kmc
	    cd kmc;\
	    ls|cpio -pdmu /usr/lib/vpm/kmc
	    -mkdir /usr/lib/vpm/kms
	    cd kms;\
	    ls|cpio -pdmu /usr/lib/vpm/kms
	   $(INS) vpmc
	   $(INSE) vpmset
	   $(INSE) dmkset
	   $(INSE) vpmsave
	   $(INSE) vpmfmt
	   $(INSE) vpmtest
	   $(INSE) vpmfsr
	   $(INSE) vpmstart

	cd scripts;\
	vpmc -mfi hdlc/kms -o ../cslapb.kms cslapb.r
	cd scripts;\
	vpmc -mfi hdlc -o ../cslapb.o cslapb.r
	chmod 444 cslapb.o cslapb.kms
	ls cslapb.o cslapb.kms|cpio -pmuv /lib

clean:
	    -rm -f *.o
		cd ratsrc;\
		make -f ratfor.mk clean
		cd plsrc;\
		make -f pl.mk clobber
		cd mplsrc;\
		make -f mpl.mk clobber

clobber:    clean
	    -rm -f pl mpl vratfor
	    -rm -f pss0 pss12 plistg
	    -rm -f vpmset dmkset vpmsave vpmfmt vpmtest vpmfsr
	-rm -f cslapb.kms


#	vpm.mk 1.3 of 10/2/81
#	@(#)vpm.mk	1.3

FRC =
INS = /etc/install -n /usr/bin
INSE = /etc/install -n /etc
INSL = /etc/install -n /lib
INSV = /etc/install -f /usr/lib/vpm
CFLAGS = -O


all:	vpmc vpmstart vpmset vpmsave vpmfmt vpmtest vpmfsr

vpmc:	pl pss0 pss12 plistg vratfor

pl:
	cd plsrc;\
	make -f pl.mk all;\
	mv pl ..

vratfor:
	cd ratsrc;\
	make -f ratfor.mk all;\
	mv ratfor ../vratfor

pss0:
	cc -O -o pss0 pass0.c

plistg:
	cc -O -o plistg plistg.c

pss12:
	cc -O -o pss12 pass12.c

vpmstart:

vpmset:

vpmsave:

vpmfmt:

vpmtest:

vpmfsr:

install:    all
	   $(INSV) pl
	   $(INSV) vratfor
	   $(INSV) pss0
	   $(INSV) pss12
	   $(INSV) plistg
	    cd bisync;\
	    ls|cpio -pdmu /usr/lib/vpm/bisync
	    cd hdlc;\
	    find . -print | cpio -pdmu /usr/lib/vpm/hdlc
	   $(INS) vpmc
	   $(INSE) vpmset
	   $(INSE) vpmsave
	   $(INSE) vpmfmt
	   $(INSE) vpmtest
	   $(INSE) vpmfsr
	   $(INSE) vpmstart

	cd scripts;\
	vpmc -mi hdlc/kms -o ../cslapb.kms cslapb.r
	cd scripts;\
	vpmc -mi hdlc -o ../cslapb.o cslapb.r
	chmod 444 cslapb.o cslapb.kms
	ls cslapb.o cslapb.kms|cpio -pmuv /lib

clean:
	    -rm -f *.o
		cd ratsrc;\
		make -f ratfor.mk clean
		cd plsrc;\
		make -f pl.mk clobber
clobber:    clean
	    -rm -f pl vratfor
	    -rm -f pss0 pss12 plistg
	    -rm -f vpmset vpmsave vpmfmt vpmtest vpmfsr
	-rm -f cslapb.kms


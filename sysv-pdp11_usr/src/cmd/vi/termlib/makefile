# sccsid = @(#) makefile 1.5@(#)
##
#
CFLAGS=	-O -DCM_N -DCM_GT -DCM_B -DCM_D

all: termcap.a install

termcap.o: ../local/uparm.h
${DESTDIR}/usr/lib/libtermcap.a termcap.a: termcap.o tgoto.o tputs.o
	ar cr termcap.a `lorder termcap.o tgoto.o tputs.o |tsort`
	cp termcap.a ${DESTDIR}/usr/lib/libtermcap.a
	chown bin ${DESTDIR}/usr/lib/libtermcap.a
	chgrp bin ${DESTDIR}/usr/lib/libtermcap.a
	-rm -f ${DESTDIR}/usr/lib/libtermlib.a
	ln ${DESTDIR}/usr/lib/libtermcap.a ${DESTDIR}/usr/lib/libtermlib.a

clean:
	-rm -f *.o
	-rm -f termcap.a

install: ${DESTDIR}/usr/lib/libtermcap.a

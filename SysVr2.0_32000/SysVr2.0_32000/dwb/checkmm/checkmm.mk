#	checkmm make file
#
# for DSL 2
#	SCCS:	@(#)checkmm.mk	1.10

OL = $(ROOT)/
INS = :
INSDIR = ${OL}usr/bin
B10 =
CFLAGS = -O ${FFLAG} ${B10}
IFLAG = -n
SOURCE = chekl.l chekmain.c
FILES = chekl.o chekmain.o
MAKE = make

compile all: checkmm
	:

checkmm:	$(FILES)
	$(CC) -s ${B10} ${IFLAG} -o checkmm $(FILES) -ll -lPW
	$(INS) checkmm $(INSDIR)
	cd $(INSDIR); chmod 755 checkmm; $(CH) chgrp bin checkmm; chown bin checkmm

install:
	${MAKE} -f checkmm.mk INS=cp ROOT=$(ROOT) ch=$(CH)

clean:
	  rm -f ${FILES}

clobber:  clean
	  rm -f checkmm

#	text devX97 make file
#
# DSL 2.
#	SCCS: @(#)devX97.mk	1.6

OL = $(ROOT)/
INS = :
CINSDIR = ${OL}usr/bin
B20 =
CFLAGS = -O -I.. ${B20}
LDFLAGS = -s
OBJECTS= char.o curve.o djd.o draw.o globals.o main.o put_page.o \
	read_desc.o spacing.o util.o
CHFILES = mplot.h structs.h
HFILES = ../dev.h ${CHFILES}
CFILES = char.c curve.c djd.c draw.c globals.c main.c put_page.c \
	read_desc.c spacing.c util.c
SOURCE = ${HFILES} ${CFILES}
SFILES = ${CHFILES} ${CFILES}

dx9700 : $(OBJECTS)
	$(CC) $(OBJECTS) -lm ${LDFLAGS} -o dx9700
	${INS} dx9700 ${CINSDIR}
	cd ${CINSDIR}; chmod 755 dx9700; $(CH) chgrp bin dx9700; chown bin dx9700

char.o : char.c structs.h ../dev.h
curve.o : curve.c structs.h ../dev.h
draw.o : draw.c structs.h mplot.h ../dev.h
globals.o : globals.c structs.h ../dev.h
main.o : main.c structs.h ../dev.h
put_page.o : put_page.c structs.h ../dev.h
read_desc.o : read_desc.c structs.h ../dev.h
spacing.o : spacing.c structs.h ../dev.h
util.o : util.c structs.h ../dev.h


install:
	${MAKE} -f devX97.mk INS=cp ROOT=$(ROOT) CH=$(CH)
	cd X97.tim10p; make -f tim10p.mk install ROOT=$(ROOT) CH=$(CH)
	cd X97.tim12p; make -f tim12p.mk install ROOT=$(ROOT) CH=$(CH)

clean:
	rm -f $(OBJECTS)
	cd X97.tim10p; make -f tim10p.mk clean
	cd X97.tim12p; make -f tim12p.mk clean

clobber:
	rm -f $(OBJECTS)
	rm -f dx9700
	cd X97.tim10p; make -f tim10p.mk clobber
	cd X97.tim12p; make -f tim12p.mk clobber

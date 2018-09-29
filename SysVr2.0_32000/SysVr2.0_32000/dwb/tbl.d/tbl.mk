#	text Development Support Library (DSL) tbl make file
#
# DSL 2
#	SCCS: @(#)tbl.mk	1.17

OL = $(ROOT)/
INS = :
CINSDIR = ${OL}usr/bin
LIST = lp
B20 =
CFLAGS = -O ${B20}
SFILES = t..c t[0-9].c t[bcefgimrstuv].c
OFILES = t0.o t1.o t2.o t3.o t4.o t5.o t6.o t7.o t8.o t9.o tb.o tc.o\
	te.o tf.o tg.o ti.o tm.o tr.o ts.o tt.o tu.o tv.o
IFLAG = -i
MAKE = make

compile all: tbl
	:

tbl:	$(OFILES) 
	$(CC) $(IFLAG) $(CFLAGS) ${FFLAG} -s -o tbl $(OFILES)
	${INS} tbl ${CINSDIR}
	cd ${CINSDIR}; chmod 755 tbl; $(CH) chgrp bin tbl; chown bin tbl

$(OFILES):: t..c
	:

install:
	${MAKE} -f tbl.mk INS=cp ROOT=$(ROOT) CH=$(CH)

clean:
	rm -f *.o

clobber: clean
	rm -f tbl

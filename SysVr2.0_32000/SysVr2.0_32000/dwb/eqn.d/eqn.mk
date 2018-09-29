#	eqn make file (text subsystem)
#
# for DSL 2
#	SCCS:  @(#)eqn.mk	2.6

OL = $(ROOT)/
INS = :
CINSDIR = ${OL}usr/bin
TINSDIR = ${OL}usr/pub
B20 =
CFLAGS = -O ${B20}
IFLAG = -n
YFLAGS = -d
SOURCE = e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
	 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
	 pile.c shift.c size.c sqrt.c text.c
OFILES =  diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
	 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
	 pile.o shift.o size.o sqrt.o text.o
FILES =  ${OFILES} e.o
MAKE = make

compile all: eqn apseqnch cateqnch eqnch
	:

eqn:	$(FILES)
	$(CC) -s ${IFLAG} ${FFLAG} -o eqn $(FILES) -ly
	$(INS) eqn $(CINSDIR)
	cd $(CINSDIR); chmod 755 eqn; $(CH) chgrp bin eqn; chown bin eqn

$(OFILES):: e.h e.def
	:

e.def:	  y.tab.h
	  -cmp -s y.tab.h e.def || cp y.tab.h e.def

y.tab.h:  e.o
	:

apseqnch:	apseqnchar
	${INS} apseqnchar ${TINSDIR}
	cd $(TINSDIR); chmod 644 apseqnchar; $(CH) chgrp bin apseqnchar; chown bin apseqnchar

cateqnch:	cateqnchar
	${INS} cateqnchar ${TINSDIR}
	cd $(TINSDIR); chmod 644 cateqnchar; $(CH) chgrp bin cateqnchar; chown bin cateqnchar

eqnch:	apseqnch cateqnchar
	if [ ${INS} != : ]; \
	then \
		rm -f $(TINSDIR)/eqnchar; \
		ln ${TINSDIR}/apseqnchar ${TINSDIR}/eqnchar; \
	fi

install:
	${MAKE} -f eqn.mk INS=cp ROOT=$(ROOT) CH=$(CH)
inseqnch:
	${MAKE} -f eqn.mk INS=cp ROOT=$(ROOT) CH=$(CH) eqnch

clean:
	  rm -f *.o y.tab.h e.def

clobber:  clean
	  rm -f eqn

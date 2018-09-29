#	makefile for pic.
#
# DSL 2.
# SCCS @(#)pic.mk	1.6


OL = $(ROOT)/
YFLAGS = -d
MOFILES = main.o boxgen.o print.o misc.o symtab.o blockgen.o circgen.o  \
	  arcgen.o linegen.o movegen.o troffgen.o textgen.o  \
	  pic2.o lineseg.o
OFILES = main.o print.o misc.o symtab.o blockgen.o boxgen.o circgen.o \
	arcgen.o linegen.o movegen.o troffgen.o textgen.o \
	pic2.o pltroff.o
CFILES = main.c print.c misc.c symtab.c blockgen.c boxgen.c circgen.c \
	arcgen.c linegen.c movegen.c troffgen.c textgen.c \
	pic2.c pltroff.c
SRCFILES = picy.y picl.l pic.h $(CFILES)
INS = :
INSDIR = $(OL)usr/bin
IFLAG = -i
LDFLAGS = -s

all:	pic

pic::	picy.o picl.o $(OFILES) 
	$(CC) -o pic $(IFLAG) $(LDFLAGS) picy.o picl.o $(OFILES) -lm
	$(INS) pic $(INSDIR)
	cd $(INSDIR); chmod 755 pic; $(CH) chgrp bin pic; chown bin pic

$(OFILES):	pic.h
picy.c:	picy.y pic.h
picl.c:	picl.l pic.h

y.tab.h:	picy.o

pic.ydef:	y.tab.h
	-cmp -s y.tab.h pic.ydef || cp y.tab.h pic.ydef

pltroff:	driver.o pltroff.o
	$(CC)  $(IFLAG) -o pltroff pltroff.o driver.o -lm

install:
	$(MAKE) -f pic.mk INS=cp ROOT=$(ROOT) CH=$(CH)

clean:
	rm -f $(OFILES) y.tab.h picy.c picl.c

clobber:	clean
	rm -f pic pltroff

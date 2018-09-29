#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)awk:awk.mk	2.6"
ROOT=
OL = /
SL = $(ROOT)/usr/src/cmd
INSDIR = $(ROOT)/usr/bin
RDIR = $(SL)/awk
YACCRM=-rm
TESTDIR = .
FRC =
INS = :
B10 =
B11 =
IFLAG = -i
CFLAGS=-O $(FFLAG)
YFLAGS=-d
LDFLAGS = -s $(IFLAG)
LIBS=	-lc_s -lm -ll
REL = current
LIST = lp
FILES=	awk.g.o       awk.lx.o b.o lib.o main.o parse.o \
	proctab.o run.o tran.o
SOURCE=	awk.g.y awk.h awk.lx.l b.c lib.c main.c parse.c \
	proctab.c run.c tran.c

all:  awk

awk:	$(FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(FILES)  $(LIBS) -o  $(TESTDIR)/awk
	$(INS)     $(TESTDIR)/awk	$(INSDIR)

$(FILES):	awk.h prevy.tab.h $(FRC)

awk.g.o:	awk.h awk.g.c

y.tab.h awk.g.c:	awk.g.y
		$(YACC) $(YFLAGS) awk.g.y
		mv y.tab.c awk.g.c

prevy.tab.h:	y.tab.h
	-cmp -s y.tab.h prevy.tab.h || (cp y.tab.h prevy.tab.h; echo change maketab)

proctab.c:	maketab
	./maketab >proctab.c

maketab:	prevy.tab.h maketab.c
	cc maketab.c -o maketab

src:	$(SOURCE) test.a awk.mk
	cp $? $(RDIR)
	touch src

get:
	for i in $(SOURCE) awk.mk; do cp $(RDIR)/$$i .; done

bin:
	cp a.out $(INSDIR)/awk
	strip $(INSDIR)/awk

profile:	$(FILES) mon.o
	$(CC) -p -i $(FILES) mon.o $(LIBS)

find:
	egrep -n "$(PAT)" *.[ylhc] awk.def

list:
	-pr WISH $(SOURCE) awk.mk tokenscript README EXPLAIN

lint:
	lint -pu b.c main.c token.c tran.c run.c lib.c parse.c -lm |\
		egrep -v '^(error|free|malloc)'

clean:
	rm -f a.out *.o t.* *temp* *.out *junk* y.tab.* ./maketab proctab.c yacc.*

diffs:
	-for i in $(SOURCE); do echo $$i:; diff $$i $(RDIR) | ind; done
lcomp:
	-rm -f  [b-z]*.o
	lcomp b.c main.c tran.c run.c lib.c parse.c proctab.c *.o $(LIBS)

FRC:

install: 
	$(MAKE) -f awk.mk INS=cp OL=$(OL) IFLAG=$(IFLAG) YACCRM=$(YACCRM) 

build:	bldmk
	get -p -r`gsid awk $(REL)` s.awk.src $(REWIRE) | ntar -d $(RDIR) -g
	cd $(RDIR) ; $(YACC) $(YFLAGS) awk.g.y
	cd $(RDIR) ; mv y.tab.c  awk.g.c ; rm -f y.tab.h

bldmk:
	get -p -r`gsid awk.km $(REL) s.awk.mk >$(RDIR)/awk.mk

listing:
	pr awk.mk $(SOURCE) | $(LIST)

clobber:	clean
	-rm -f awk.g.c prevy.tab.*
	-rm  -f  $(TESTDIR)/awk

delete:	clobber
	rm -f $(SOURCE)


OL = /
SL = /usr/src/cmd
INSDIR = $(OL)usr/bin
RDIR = $(SL)/awk
YACCRM=-rm
TESTDIR = .
FRC =
INS = :
B10 =
B11 =
IFLAG = -i
CFLAGS=-O
YFLAGS=-d
LDFLAGS = -s $(IFLAG) $(FFLAGS)
REL = current
LIST = lp
FILES=awk.lx.o b.o main.o token.o tran.o lib.o run.o parse.o proctab.o freeze.o
SOURCE=EXPLAIN README awk.def awk.g.y awk.h awk.lx.l b.c lib.c main.c parse.c \
	makeprctab.c freeze.c\
	run.c token.c tokenscript tran.c

all:  awk

awk:	$(FILES) awk.g.o
	$(CC) $(LDFLAGS) awk.g.o  $(FILES)  -lm -o  $(TESTDIR)/awk
	$(INS)     $(TESTDIR)/awk	$(INSDIR)

y.tab.h:	awk.g.o

awk.h:	y.tab.h
	-cp y.tab.h awk.h

$(FILES):	awk.h awk.def $(FRC)

token.c:	awk.h tokenscript
	ed - <tokenscript

src:	$(SOURCE) test.a tokenscript makefile README
	cp $? /usr/src/cmd/awk
	touch src

get:
	for i in $(SOURCE) awk.mk tokenscript README; do cp /usr/src/cmd/awk/$$i .; done

bin:
	cp a.out /usr/bin/awk
	strip /usr/bin/awk

profile:	awk.g.o $(FILES) mon.o
	$(CC) -p -i awk.g.o $(FILES) mon.o -lm

find:
	egrep -n "$(PAT)" *.[ylhc] awk.def

list:
	-pr WISH $(SOURCE) awk.mk tokenscript README EXPLAIN

lint:
	lint -spu b.c main.c token.c tran.c run.c lib.c parse.c -lm |\
		egrep -v '^(error|free|malloc)'

proctab.c:	./makeprctab
	./makeprctab >proctab.c

./makeprctab:	awk.h makeprctab.o token.o
	$(CC) -o ./makeprctab makeprctab.o token.o

clean:
	rm -f a.out *.o t.* *temp* *.out *junk* y.tab.* awk.h ./makeprctab proctab.c

diffs:
	-for i in $(SOURCE); do echo $$i:; diff $$i /usr/src/cmd/awk | ind; done
lcomp:
	-rm -f  [b-z]*.o
	lcomp b.c main.c token.c tran.c run.c lib.c parse.c freeze.c proctab.c   *.o -lm

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

listmk:
	pr awk.mk | $(LIST)

edit:
	get -e -p s.awk.src |ntar -g

delta:
	ntar -p $(SOURCE) >awk.src
	delta s.awk.src
	rm -f $(SOURCE)

mkedit:		;	get -e s.awk.mk
mkdelta:	;	delta s.awk.mk

clobber:	clean
	-rm  -f  $(TESTDIR)/awk

delete:	clobber
	rm -f $(SOURCE)


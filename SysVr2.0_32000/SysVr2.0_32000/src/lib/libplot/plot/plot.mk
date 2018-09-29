#	libplot.a make file
#	SCCS:	@(#)plot.mk	1.4

OL = /
SL = /usr/src/lib
INSDIR = $(OL)usr/lib
RDIR = $(SL)/libplot/plot
REL = current
INS = :
LIST = lp
TESTDIR = .
CFLAGS = -O
SOURCE = arc.c box.c circle.c close.c cont.c dot.c erase.c label.c \
	 line.c linmod.c move.c open.c point.c putsi.c space.c
OFILES = arc.o box.o circle.o close.o cont.o dot.o erase.o label.o \
	 line.o linmod.o move.o open.o point.o putsi.o space.o
MAKE = make

all:	libplot.a
	:

libplot.a:	$(OFILES)
	ar r $(TESTDIR)/libplot.a $(OFILES)
	$(INS) $(TESTDIR)/libplot.a $(INSDIR)
	-chmod 664 $(INSDIR)/libplot.a
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin libplot.a; chgrp bin libplot.a; \
	 fi

install:
	$(MAKE) -f plot.mk INS=cp OL=$(OL)

build:	bldmk
	get -p -r`gsid plot $(REL)` s.plot.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:
	get -p -r`gsid plot.mk $(REL)` s.plot.mk > $(RDIR)/plot.mk

listing:
	pr plot.mk $(SOURCE) | $(LIST)
listmk:
	pr plot.mk | $(LIST)

edit:
	get -p -e s.plot.src | ntar -g

delta:
	ntar -p $(SOURCE) > plot.src
	delta s.plot.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.plot.mk
mkdelta: ;  delta s.plot.mk

clean:
	-rm -f $(OFILES)

clobber: clean
	-rm -f $(TESTDIR)/libplot.a

delete:	clobber
	rm -f $(SOURCE)

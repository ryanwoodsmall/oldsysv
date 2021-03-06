#	libvt0.a make file
#	SCCS:	@(#)vt0.mk	1.3

OL = /
SL = /usr/src/lib
INSDIR = $(OL)usr/lib
RDIR = $(SL)/libplot/vt0
REL = current
INS = :
LIST = lp
TESTDIR = .
CFLAGS = -O
SOURCE = arc.c box.c circle.c close.c dot.c erase.c frame.c label.c \
	 line.c linmod.c move.c open.c point.c space.c subr.c
OFILES = arc.o box.o circle.o close.o dot.o erase.o frame.o label.o \
	 line.o linmod.o move.o open.o point.o space.o subr.o
MAKE = make

all:	libvt0.a
	:

libvt0.a:	$(OFILES)
	ar r $(TESTDIR)/libvt0.a $(OFILES)
	$(INS) $(TESTDIR)/libvt0.a $(INSDIR)
	chmod 664 $(INSDIR)/libvt0.a
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin libvt0.a; chgrp bin libvt0.a; \
	 fi

install:
	$(MAKE) -f vt0.mk INS=cp OL=$(OL)

build:	bldmk
	get -p -r`gsid vt0 $(REL)` s.vt0.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:
	get -p -r`gsid vt0.mk $(REL)` s.vt0.mk > $(RDIR)/vt0.mk

listing:
	pr vt0.mk $(SOURCE) | $(LIST)
listmk:
	pr vt0.mk | $(LIST)

edit:
	get -p -e s.vt0.src | ntar -g

delta:
	ntar -p $(SOURCE) > vt0.src
	delta s.vt0.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.vt0.mk
mkdelta: ;  delta s.vt0.mk

clean:
	-rm -f $(OFILES)

clobber: clean
	-rm -f $(TESTDIR)/libvt0.a

delete:	clobber
	rm -f $(SOURCE)

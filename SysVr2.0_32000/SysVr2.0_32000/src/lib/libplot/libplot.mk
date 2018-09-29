#	libplot make file
#	SCCS:	@(#)libplot.mk	1.2

OL = /
SL = /usr/src/lib
INSDIR = $(OL)usr/lib
RDIR = $(SL)/libplot
INS = :
REL = current
LIST = lp
MAKE = make

all:	libplot.a lib300.a lib300s.a lib4014.a lib450.a vt0.a
	:

libplot.a:
	cd plot; $(MAKE) -f plot.mk INS=$(INS) OL=$(OL) install

lib300.a:
	cd t300; $(MAKE) -f t300.mk INS=$(INS) OL=$(OL) install

lib300s.a:
	cd t300s; $(MAKE) -f t300s.mk INS=$(INS) OL=$(OL) install

lib4014.a:
	cd t4014; $(MAKE) -f t4014.mk INS=$(INS) OL=$(OL) install

lib450.a:
	cd t450; $(MAKE) -f t450.mk INS=$(INS) OL=$(OL) install

vt0.a:
	cd vt0; $(MAKE) -f vt0.mk INS=$(INS) OL=$(OL) install

install:
	$(MAKE) -f libplot.mk INS=cp OL=$(OL)

build:	bldmk
	cd plot; $(MAKE) -f plot.mk SL=$(SL) REL=$(REL) REWIRE='$(REWIRE)' build
	cd t300; $(MAKE) -f t300.mk SL=$(SL) REL=$(REL) REWIRE='$(REWIRE)' build
	cd t300s; $(MAKE) -f t300s.mk SL=$(SL) REL=$(REL) REWIRE='$(REWIRE)' build
	cd t4014; $(MAKE) -f t4014.mk SL=$(SL) REL=$(REL) REWIRE='$(REWIRE)' build
	cd t450; $(MAKE) -f t450.mk SL=$(SL) REL=$(REL) REWIRE='$(REWIRE)' build
	cd vt0; $(MAKE) -f vt0.mk SL=$(SL) REL=$(REL) REWIRE='$(REWIRE)' build
bldmk:
	get -p -r`gsid libplot.mk $(REL)` s.libplot.mk > $(RDIR)/libplot.mk

listing:	listmk
	cd plot; $(MAKE) -f plot.mk LIST='$(LIST)' listing
	cd t300; $(MAKE) -f t300.mk LIST='$(LIST)' listing
	cd t300s; $(MAKE) -f t300s.mk LIST='$(LIST)' listing
	cd t4014; $(MAKE) -f t4014.mk LIST='$(LIST)' listing
	cd t450; $(MAKE) -f t450.mk LIST='$(LIST)' listing
	cd vt0; $(MAKE) -f vt0.mk LIST='$(LIST)' listing
listmk:
	pr libplot.mk | $(LIST)

mkedit:  ;  get -e s.libplot.mk
mkdelta: ;  delta s.libplot.mk

clean:
	cd plot; $(MAKE) -f plot.mk clean
	cd t300; $(MAKE) -f t300.mk clean
	cd t300s; $(MAKE) -f t300s.mk clean
	cd t4014; $(MAKE) -f t4014.mk clean
	cd t450; $(MAKE) -f t450.mk clean
	cd vt0; $(MAKE) -f vt0.mk clean

clobber:
	cd plot; $(MAKE) -f plot.mk clobber
	cd t300; $(MAKE) -f t300.mk clobber
	cd t300s; $(MAKE) -f t300s.mk clobber
	cd t4014; $(MAKE) -f t4014.mk clobber
	cd t450; $(MAKE) -f t450.mk clobber
	cd vt0; $(MAKE) -f vt0.mk clobber

delete:
	cd plot; $(MAKE) -f plot.mk delete
	cd t300; $(MAKE) -f t300.mk delete
	cd t300s; $(MAKE) -f t300s.mk delete
	cd t4014; $(MAKE) -f t4014.mk delete
	cd t450; $(MAKE) -f t450.mk delete
	cd vt0; $(MAKE) -f vt0.mk delete

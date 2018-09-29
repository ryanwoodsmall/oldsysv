#	tplot make file
#	SCCS:	@(#)tplot.mk	1.5

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
INSDIR = $(OL)usr/bin
INSLIB = $(OL)usr/lib
RDIR = $(SL)/tplot
REL = current
LIST = lp
INS = :
IFLAG = -i
CFLAGS = -O
LDFLAGS = -s $(FFLAG)
SOURCE = banner.c driver.c tplot.sh vplot.c
MAKE = make

all: t4014 t300 t300s t450 vplot tplot

t4014: driver.o
	$(CC) $(LDFLAGS) -o t4014 driver.o -l4014 -lm
	$(INS) $(INSLIB) t4014

t300:	driver.o
	$(CC) $(LDFLAGS) -o t300 driver.o -l300 -lm
	$(INS) $(INSLIB) t300

t300s: driver.o
	$(CC) $(LDFLAGS) -o t300s driver.o -l300s -lm
	$(INS) $(INSLIB) t300s

t450:	driver.o
	$(CC) $(LDFLAGS) -o t450 driver.o -l450 -lm
	$(INS) $(INSLIB) t450

vplot:	vplot.o banner.o
	$(CC) $(LDFLAGS) $(IFLAG) -o vplot vplot.o banner.o
	$(INS) $(INSLIB) vplot

tplot:	tplot.sh
	cp tplot.sh tplot
	$(INS) $(INSDIR) tplot

install:
	 $(MAKE) -f tplot.mk INS="install -f" OL=$(OL) IFLAG=$(IFLAG)

build:	bldmk
	get -p -r`gsid tplot current` s.tplot.src $(REWIRE) | ntar -d $(RDIR) -g

bldmk:
	get -p -r`gsid tplot.mk current` s.tplot.mk > $(RDIR)/tplot.mk

listing:
	pr tplot.mk $(SOURCE) | $(LIST)

listmk:
	pr tplot.mk | $(LIST)

edit:
	get -e -p s.tplot.src | ntar -g

delta:
	ntar -p $(SOURCE) > tplot.src
	delta s.tplot.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.tplot.mk
mkdelta: ;  delta s.tplot.mk

clean:
	 -rm -f *.o

clobber: clean
	 -rm -f t4014 t300 t300s t450 vplot tplot

delete:	clobber
	-rm -f $(SOURCE)

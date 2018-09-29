#	tplot make file
#	SCCS:	@(#)tplot.mk	1.4

OL = /
SL = /usr/src/cmd
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
	$(INS) t4014 $(INSLIB)
	chmod 775 $(INSLIB)/t4014
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSLIB); chown bin t4014; chgrp bin t4014; \
	 fi

t300:	driver.o
	$(CC) $(LDFLAGS) -o t300 driver.o -l300 -lm
	$(INS) t300 $(INSLIB)
	chmod 775 $(INSLIB)/t300
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSLIB); chown bin t300; chgrp bin t300; \
	 fi

t300s: driver.o
	$(CC) $(LDFLAGS) -o t300s driver.o -l300s -lm
	$(INS) t300s $(INSLIB)
	chmod 775 $(INSLIB)/t300s
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSLIB); chown bin t300s; chgrp bin t300s; \
	 fi

t450:	driver.o
	$(CC) $(LDFLAGS) -o t450 driver.o -l450 -lm
	$(INS) t450 $(INSLIB)
	chmod 775 $(INSLIB)/t450
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSLIB); chown bin t450; chgrp bin t450; \
	 fi

vplot:	vplot.o banner.o
	$(CC) $(LDFLAGS) $(IFLAG) -o vplot vplot.o banner.o
	$(INS) vplot $(INSLIB)
	chmod 775 $(INSLIB)/vplot
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSLIB); chown bin vplot; chgrp bin vplot; \
	 fi

tplot:	tplot.sh
	cp tplot.sh tplot
	$(INS) tplot $(INSDIR)
	chmod 775 $(INSDIR)/tplot
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin tplot; chgrp bin tplot; \
	 fi

install:
	 $(MAKE) -f tplot.mk INS=cp OL=$(OL) IFLAG=$(IFLAG)

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

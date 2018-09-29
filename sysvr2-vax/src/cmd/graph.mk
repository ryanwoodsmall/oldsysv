#	graph make file
#	SCCS:	@(#)graph.mk	1.3

OL = /
SL = /usr/src/cmd
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid graph $(REL)`
MKSID = -r`gsid graph.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
IFLAG = -i
CFLAGS = -O
LDFLAGS = -s $(IFLAG) $(FFLAG)
SOURCE = graph.c
MAKE = make

compile all: graph
	:

graph:
	$(CC) $(CFLAGS) $(LDFLAGS) -o graph graph.c -lplot -lm
	$(INS) graph $(INSDIR)
	chmod 775 $(INSDIR)/graph
	@if [ "$(OL)" = "/" ]; \
		then cd $(INSDIR); chown bin graph; chgrp bin graph; \
	 fi

install:
	$(MAKE) -f graph.mk INS=cp OL=$(OL) IFLAG=$(IFLAG)

build:	bldmk
	get -p $(CSID) s.graph.c $(REWIRE) > $(RDIR)/graph.c
bldmk:  ;  get -p $(MKSID) s.graph.mk > $(RDIR)/graph.mk

listing:
	pr graph.mk $(SOURCE) | $(LIST)
listmk: ;  pr graph.mk | $(LIST)

edit:
	get -e s.graph.c

delta:
	delta s.graph.c

mkedit:  ;  get -e s.graph.mk
mkdelta: ;  delta s.graph.mk

clean:
	:

clobber:  clean
	  rm -f graph

delete:	clobber
	rm -f $(SOURCE)

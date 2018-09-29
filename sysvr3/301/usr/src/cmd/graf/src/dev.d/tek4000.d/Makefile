#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/dev.d/tek4000.d/Makefile	1.7"
###
### Standard Constants 
###

ROOT     =
LROOT    =
BIN      = $(ROOT)/$(LROOT)/usr/bin/graf

INC	 = $(ROOT)/usr/include

CC       = cc
LDFLAGS  = -lm -i -s
CFLAGS   = -O -I$(INC) $(FFLAG)

GRAFMAKE = $(MAKE) ROOT=$(ROOT) LROOT=$(LROOT) BIN=$(BIN)

MAINS = erase hardcopy tekset

OBJECTS =  erase.o hardcopy.o tekset.o

SOURCES =  erase.c hardcopy.c tekset.c

ALL:		$(MAINS) teklib ged td 

erase:		erase.o	
	$(CC) $(CFLAGS)  -o erase  erase.o   $(LDFLAGS)

hardcopy:	hardcopy.o 
	$(CC) $(CFLAGS)  -o hardcopy  hardcopy.o   $(LDFLAGS)

tekset:		tekset.o 
	$(CC) $(CFLAGS)  -o tekset  tekset.o   $(LDFLAGS)

ged:
	cd ged.d; $(GRAFMAKE)

td:
	cd td.d; $(GRAFMAKE)

teklib: 
	cd lib; $(GRAFMAKE)

erase.o:	 $(INC)/stdio.h 

hardcopy.o:	 

tekset.o:	 $(INC)/stdio.h 

clean:
	rm -f $(OBJECTS)
	cd ged.d; $(GRAFMAKE) clean
	cd td.d; $(GRAFMAKE) clean
	cd lib; $(GRAFMAKE) clean
	
clobber:	
	rm -f $(OBJECTS) $(MAINS)
	cd ged.d; $(GRAFMAKE) clobber
	cd td.d; $(GRAFMAKE) clobber
	cd lib; $(GRAFMAKE) clobber

install      : $(MAINS)
	for i in $(MAINS) ; do cpset $$i $(BIN) ; done
	cd lib; $(GRAFMAKE) install
	cd ged.d; $(GRAFMAKE) install
	cd td.d; $(GRAFMAKE) install
	
listings     : $(SOURCES)
	pr -h "     tek4000.d     " $(SOURCES) | lp
	cd ged.d; $(GRAFMAKE) listings
	cd td.d; $(GRAFMAKE) listings

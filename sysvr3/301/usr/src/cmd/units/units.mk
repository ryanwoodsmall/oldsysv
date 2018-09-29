#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)units:units.mk	1.6"

#	Makefile for units

ROOT =

DIR = $(ROOT)/usr/bin
LIBDIR = $(ROOT)/usr/lib

INC = $(ROOT)/usr/include

LDFLAGS = $(FFLAG) $(IFLAG)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = units.mk

MAINS = units

LIB = unittab

OBJECTS =  units.o

SOURCES =  units.c unittab

ALL:		$(MAINS) $(LIB)

units:		units.o	
	$(CC) $(CFLAGS)  -o units  units.o   $(LDFLAGS)


units.o:	 $(INC)/stdio.h 

GLOBALINCS = $(INC)/stdio.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)


all : ALL

install: ALL
	cpset $(MAINS) $(DIR)
	cpset $(LIB) $(LIBDIR) 664

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort
	@echo $(LIBDIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'
	@echo $(LIB)  |  tr ' ' '\012'  | \
	sed 's;^;$(LIBDIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS) $(LIB)

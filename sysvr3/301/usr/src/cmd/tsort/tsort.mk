#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)tsort:tsort.mk	1.1.1.3"

#	Makefile for tsort

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -lgen

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#
# Generated by makefile 1.46

MAKEFILE = tsort.mk


MAINS = tsort

OBJECTS =  tsort.o

SOURCES =  tsort.c

ALL:		$(MAINS)

tsort:		tsort.o	
	$(CC) $(CFLAGS)  -o tsort  tsort.o   $(LDFLAGS)


tsort.o:	 errmsg.h $(INC)/stdio.h \
		 $(INC)/sys/types.h $(INC)/stdio.h \
		 $(INC)/string.h 

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	cpset $(MAINS) $(DIR)

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(GLOBALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)

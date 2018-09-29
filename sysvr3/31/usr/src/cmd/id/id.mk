#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)id:id.mk	1.2"

#	Makefile for id

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(LDLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = id.mk

MAINS = id

OBJECTS =  id.o

SOURCES =  id.c

ALL:		$(MAINS)

id:	id.o
	$(CC) $(CFLAGS) -o id id.o $(LDFLAGS)

id.o:		 $(INC)/stdio.h \
		 $(INC)/pwd.h \
		 $(INC)/grp.h 

GLOBALINCS = $(INC)/grp.h \
	$(INC)/pwd.h \
	$(INC)/stdio.h 


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
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)

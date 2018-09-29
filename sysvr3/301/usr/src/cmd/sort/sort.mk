#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sort:sort.mk	1.1"

#	Makefile for sort 

ROOT =

DIR = $(ROOT)/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(LDLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#
# Generated by makefile 1.47

MAKEFILE = sort.mk


MAINS = sort

OBJECTS =  sort.o

SOURCES =  sort.c

ALL:		$(MAINS)

sort:		sort.o 
	$(CC) $(CFLAGS)  -o sort  sort.o   $(LDFLAGS)


sort.o:		 $(INC)/stdio.h $(INC)/ctype.h \
		 $(INC)/signal.h $(INC)/sys/signal.h \
		 $(INC)/sys/types.h $(INC)/sys/stat.h \
		 $(INC)/values.h 

GLOBALINCS = $(INC)/ctype.h $(INC)/signal.h $(INC)/stdio.h \
	$(INC)/sys/signal.h $(INC)/sys/stat.h \
	$(INC)/sys/types.h $(INC)/values.h 


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

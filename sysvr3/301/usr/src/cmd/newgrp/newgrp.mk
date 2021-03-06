#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)newgrp:newgrp.mk	1.2"

#	Makefile for <newgrp>

ROOT =

DIR = $(ROOT)/bin

INC = $(ROOT)/usr/include

LDFLAGS = -lcrypt $(LDLIBS)

CFLAGS = -O -s -I$(INC)

STRIP = strip

SIZE = size

#top#
# Generated by makefile 1.47

MAKEFILE = newgrp.mk


MAINS = newgrp

OBJECTS =  newgrp.o

SOURCES =  newgrp.c

ALL:		$(MAINS)

newgrp:		newgrp.o 
	$(CC) $(CFLAGS)  -o newgrp  newgrp.o   $(LDFLAGS)


newgrp.o:	 $(INC)/stdio.h $(INC)/pwd.h \
		 $(INC)/grp.h 

GLOBALINCS = $(INC)/grp.h $(INC)/pwd.h $(INC)/stdio.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	cpset $(MAINS) $(DIR) 04555 root sys

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

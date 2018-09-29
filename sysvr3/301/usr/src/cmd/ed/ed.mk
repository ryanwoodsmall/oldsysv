#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ed:ed.mk	1.18"

#	Makefile for ed

ROOT =

DIR = $(ROOT)/bin

INC = $(ROOT)/usr/include

IFLAG = -n
LDFLAGS = -s -lcrypt -lgen $(IFLAG) $(LDLIBS)

B10 =
CFLAGS = -O -I$(INC) -DCRYPT $(B10)

STRIP = strip

SIZE = size

MAKEFILE = ed.mk

MAINS = ed

OBJECTS =  ed.o ed.s

SOURCES =  ed.c edfun

ALL:		$(MAINS)

ed:		ed.o 
	$(CC) $(CFLAGS)  -o ed  ed.o   $(LDFLAGS)


ed.o:		 ed.s \
	$(INC)/regexp.h $(INC)/stdio.h \
		 $(INC)/signal.h $(INC)/sys/signal.h \
		 $(INC)/sys/types.h $(INC)/sys/stat.h \
		 $(INC)/termio.h $(INC)/sys/termio.h \
		 $(INC)/ustat.h $(INC)/setjmp.h 
	if pdp11; then sh ./edfun ed.s; else true; fi
	$(CC) -c ed.s

ed.s:	ed.c
	$(CC) -S $(CFLAGS) ed.c

GLOBALINCS = $(INC)/regexp.h $(INC)/setjmp.h \
	$(INC)/signal.h $(INC)/stdio.h $(INC)/sys/signal.h \
	$(INC)/sys/stat.h $(INC)/sys/termio.h \
	$(INC)/sys/types.h $(INC)/termio.h $(INC)/ustat.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)


all : ALL

install: ALL
	cpset $(MAINS) $(DIR)
	rm -f $(DIR)/red
	ln $(DIR)/ed $(DIR)/red

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

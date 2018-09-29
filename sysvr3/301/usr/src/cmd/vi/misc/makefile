#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vi:misc/makefile	1.10"

#	Makefile for misc directory for vi

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#Must use cc not $(CC) for xstr and mkstr

MAKEFILE = makefile

MAINS = ctags fold mkstr xstr cxref

OBJECTS =  ctags.o fold.o mkstr.o xstr.o

SOURCES =  ctags.c fold.c mkstr.c xstr.c cxref.sh

ALL:		$(MAINS)

ctags:		ctags.o	
	$(CC) $(CFLAGS)  -o ctags  ctags.o   $(LDFLAGS)

fold:		fold.o 
	$(CC) $(CFLAGS)  -o fold  fold.o   $(LDFLAGS)

mkstr:		mkstr.c	
	cc -O mkstr.c -o mkstr $(LDFLAGS)

xstr:		xstr.c 
	cc -O xstr.c -o xstr  $(LDFLAGS)


ctags.o:	 $(INC)/stdio.h $(INC)/ctype.h 

fold.o:		 $(INC)/stdio.h 

mkstr.o:	 $(INC)/stdio.h $(INC)/sys/types.h \
		 $(INC)/sys/stat.h 

xstr.o:		 $(INC)/stdio.h $(INC)/ctype.h \
		 $(INC)/sys/types.h $(INC)/signal.h	\
		 $(INC)/sys/signal.h 

GLOBALINCS = $(INC)/ctype.h $(INC)/signal.h $(INC)/stdio.h \
	$(INC)/sys/signal.h $(INC)/sys/stat.h \
	$(INC)/sys/types.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	cpset ctags $(DIR)

size: ctags
	$(SIZE) ctags

strip: ALL
	$(STRIP) $(MAINS)


partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo ctags  |  tr ' ' '\012' | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) READ_ME del.h.cb -o $(OBJECTS) $(MAINS)

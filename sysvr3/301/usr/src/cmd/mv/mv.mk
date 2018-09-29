#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mv:mv.mk	1.16"

#	Makefile for mv/cp/ln

ROOT =

DIR = $(ROOT)/bin
LIBDIR = $(ROOT)/usr/lib

INC = $(ROOT)/usr/include

LDFLAGS = -s $(IFLAG)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

LIST = lp

MAKEFILE = mv.mk

MAINS = mv ln cp
LIBMAIN = mv_dir

OBJECTS =  mv.o mv_dir.o

SOURCES =  mv.c mv_dir.c

ALL:		$(MAINS) $(LIBMAIN)

compile: mv.o
	$(CC) $(CFLAGS)  -o ln  mv.o   $(LDFLAGS)

cp:		compile
	cpset ln cp

ln:		compile

mv:		compile 
	cpset ln mv

mv_dir:		mv_dir.o 
	$(CC) $(CFLAGS)  -o mv_dir  mv_dir.o   $(LDFLAGS) 


mv.o:		 $(INC)/stdio.h $(INC)/sys/types.h \
		 $(INC)/sys/stat.h $(INC)/errno.h \
		 $(INC)/sys/errno.h 

mv_dir.o:	 $(INC)/signal.h $(INC)/sys/signal.h \
		 $(INC)/stdio.h $(INC)/sys/types.h \
		 $(INC)/sys/stat.h $(INC)/errno.h \
		 $(INC)/sys/errno.h 

GLOBALINCS = $(INC)/errno.h $(INC)/signal.h $(INC)/stdio.h \
	$(INC)/sys/errno.h $(INC)/sys/signal.h \
	$(INC)/sys/stat.h $(INC)/sys/types.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS) $(LIBMAIN)


all : ALL

install: ALL
	cpset ln $(DIR) 0775
	cpset $(LIBMAIN) $(LIBDIR) 04755 root
	/bin/ln $(ROOT)/bin/ln $(ROOT)/bin/mv
	/bin/ln $(ROOT)/bin/ln $(ROOT)/bin/cp

size: ALL
	$(SIZE) ln $(LIBMAIN)

strip: ALL
	$(STRIP) ln $(LIBMAIN)

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort
	@echo $(LIBDIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS) |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'
	@echo $(LIBMAIN)  |  tr ' ' '\012'  | \
	sed 's;^;$(LIBDIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS) $(LIBMAIN)

listing:
	pr mv.mk $(SOURCE) | $(LIST)

listmk: 
	pr mv.mk | $(LIST)

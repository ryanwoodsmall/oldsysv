#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libplot:plot/plot.mk	1.7"

#	Makefile for plot library

ROOT =
LROOT =

DIR = $(ROOT)/$(LROOT)/usr/lib
USRLIB = $(DIR)

INC = $(ROOT)/usr/include

LDFLAGS =

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

LIST = lp

#top#
# Generated by makefile 1.47

MAKEFILE = plot.mk


LIBRARY = libplot.a

OBJECTS =  arc.o box.o circle.o close.o cont.o dot.o erase.o label.o line.o \
	linmod.o move.o open.o point.o putsi.o space.o

SOURCES =  arc.c box.c circle.c close.c cont.c dot.c erase.c label.c line.c \
	linmod.c move.c open.c point.c putsi.c space.c

ALL:		$(LIBRARY)

$(LIBRARY):	$(LIBRARY)(space.o) $(LIBRARY)(putsi.o)	\
		$(LIBRARY)(point.o) $(LIBRARY)(open.o) $(LIBRARY)(move.o) \
		$(LIBRARY)(linmod.o) $(LIBRARY)(line.o)	\
		$(LIBRARY)(label.o) $(LIBRARY)(erase.o)	$(LIBRARY)(dot.o) \
		$(LIBRARY)(cont.o) $(LIBRARY)(close.o) \
		$(LIBRARY)(circle.o) $(LIBRARY)(box.o) $(LIBRARY)(arc.o) 


$(LIBRARY)(arc.o): $(INC)/stdio.h	

$(LIBRARY)(box.o): 

$(LIBRARY)(circle.o):	 $(INC)/stdio.h 

$(LIBRARY)(close.o):	 $(INC)/stdio.h 

$(LIBRARY)(cont.o): $(INC)/stdio.h 

$(LIBRARY)(dot.o): $(INC)/stdio.h	

$(LIBRARY)(erase.o):	 $(INC)/stdio.h 

$(LIBRARY)(label.o):	 $(INC)/stdio.h 

$(LIBRARY)(line.o): $(INC)/stdio.h 

$(LIBRARY)(linmod.o):	 $(INC)/stdio.h 

$(LIBRARY)(move.o): $(INC)/stdio.h 

$(LIBRARY)(open.o): 

$(LIBRARY)(point.o):	 $(INC)/stdio.h 

$(LIBRARY)(putsi.o):	 $(INC)/stdio.h 

$(LIBRARY)(space.o):	 $(INC)/stdio.h 

GLOBALINCS = $(INC)/stdio.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(LIBRARY)

newmakefile:
	makefile -m -f $(MAKEFILE) -L $(LIBRARY)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	cpset $(LIBRARY) $(USRLIB)

size: ALL
	$(SIZE) $(LIBRARY)

strip: ALL

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(USRLIB) | tr ' ' '\012' | sort

product:
	@echo $(LIBRARY)  |  tr ' ' '\012'  | \
	sed 's;^;$(USRLIB)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(LIBRARY)

listing:
	pr -n $(MAKEFILE) $(SOURCES) | $(LIST)

listmk:
	pr -n $(MAKEFILE) | $(LIST)

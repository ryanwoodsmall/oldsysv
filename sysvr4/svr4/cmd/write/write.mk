#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)write:write.mk	1.9.1.1"


#	Makefile for write 

ROOT =

DIR = $(ROOT)/usr/bin

INS = install

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#
# Generated by makefile 1.47

MAKEFILE = write.mk


MAINS = write

OBJECTS =  write.o

SOURCES =  write.c

ALL:		$(MAINS)

write:		write.o	
	$(CC) $(CFLAGS)  -o write  write.o   $(LDFLAGS)


write.o:	 $(INC)/stdio.h $(INC)/signal.h \
		 $(INC)/sys/signal.h $(INC)/sys/types.h \
		 $(INC)/sys/stat.h $(INC)/sys/utsname.h \
		 $(INC)/stdlib.h $(INC)/unistd.h $(INC)/time.h \
		 $(INC)/utmp.h $(INC)/pwd.h	$(INC)/fcntl.h

GLOBALINCS = $(INC)/fcntl.h $(INC)/pwd.h $(INC)/signal.h \
	$(INC)/stdio.h $(INC)/sys/signal.h \
	$(INC)/sys/stat.h $(INC)/sys/types.h \
	$(INC)/sys/utsname.h $(INC)/utmp.h \
		 $(INC)/stdlib.h $(INC)/unistd.h $(INC)/time.h


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	$(INS) -f $(DIR) -m 02555 -u bin -g tty write

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

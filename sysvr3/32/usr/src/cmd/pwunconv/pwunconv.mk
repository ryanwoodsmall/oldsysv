#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pwunconv:pwunconv.mk	1.1"

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

INS = install

CFLAGS = -O -I$(INC)

#	Common Libraries and -l<lib> flags.
LDFLAGS = -s -lsec $(LDLIBS)

STRIP = strip

MAKEFILE = pwunconv.mk

MAINS = pwunconv

OBJECTS =  pwunconv.o

SOURCES =  pwunconv.c

ALL:		$(MAINS)

pwunconv:		pwunconv.o	
	$(CC) $(CFLAGS)  -o pwunconv  pwunconv.o   $(LDFLAGS)


pwunconv.o:	 $(INC)/sys/types.h $(INC)/utmp.h	\
		 $(INC)/signal.h $(INC)/sys/stat.h	\
		 $(INC)/ctype.h	$(INC)/shadow.h $(INC)/unistd.h \
		 $(INC)/string.h $(INC)/pwd.h $(INC)/stdio.h 

clean:
	rm -f $(OBJECTS)
	
clobber:	
	rm -f $(OBJECTS) $(MAINS)

install:	$(MAINS) $(DIR)
	$(INS) -f $(DIR) -m 0500 -u root -g sys $(MAINS)

strip:
	$(STRIP) $(MAINS)

remove:
	cd $(DIR);  rm -f $(MAINS)

$(DIR):
	mkdir $(DIR);  $(CH)chmod 755 $(DIR);  $(CH)chown bin $(DIR)

partslist:
	@echo $(MAKEFILE) $(LOCALINCS) $(SOURCES)  |  tr ' ' '\012'  |  sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
		sed -e 's;^;$(DIR)/;' -e 's;//*;/;g'

productdir:
	@echo $(DIR)

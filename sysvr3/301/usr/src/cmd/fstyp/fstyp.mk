#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fstyp:fstyp.mk	1.3"
#
#	Makefile for the fstyp component
#

ROOT =
DIR1 = $(ROOT)/etc
DIR2 = $(ROOT)/etc/fstyp.d


INC = $(ROOT)/usr/include
LDFLAGS =
LDLIBS =
CFLAGS = -O -s -I$(INC)
STRIP = strip

OBJECTS =  s5fstyp.o

all: fstyp s5fstyp

s5fstyp:  s5fstyp.o
	$(CC) $(CFLAGS)  -o s5fstyp  s5fstyp.o  $(LDFLAGS) $(LDLIBS)


s5fstyp.o:	 $(INC)/stdio.h $(INC)/fcntl.h \
		 $(INC)/sys/types.h $(INC)/sys/stat.h \
		 $(INC)/sys/fstyp.h $(INC)/sys/ustat.h \
		 $(INC)/sys/ino.h	$(INC)/sys/param.h \
		 $(INC)/sys/fs/s5param.h $(INC)/sys/fs/s5filsys.h

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f fstyp s5fstyp

install: all dir
	cpset fstyp $(DIR1) 00555 root sys
	cpset s5fstyp $(DIR2) 00555 root sys

dir:
	if [ ! -d $(DIR2) ] ;\
	then \
		mkdir $(DIR2) ;\
	fi

strip: all
	$(STRIP) s5fstyp

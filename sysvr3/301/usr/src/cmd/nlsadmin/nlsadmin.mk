#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlsadmin:nlsadmin.mk	1.5"

ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include
INS = :
CFLAGS = -O -Du3b2 -Uu3b -Uvax -Updp11 -Uu3b5 -US4 -I$(INC)
LDFLAGS = -s
SOURCE = main.c util.c nlsadmin.h nlsaddr.c nstoa.c nlsstr.c
OBJECTS = main.o util.o nstoa.o nlsaddr.o nlsstr.o
FRC =

all: nlsadmin

nlsadmin: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TESTDIR)/nlsadmin $(LDFLAGS) $(OBJECTS)

install: all
	install -n $(INSDIR) $(TESTDIR)/nlsadmin

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(TESTDIR)/nlsadmin
FRC:


main.o: $(INC)/sys/types.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/signal.h \
	$(INC)/string.h \
	$(INC)/fcntl.h \
	$(INC)/errno.h \
	nlsadmin.h 
util.o: $(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/string.h \
	$(INC)/fcntl.h \
	nlsadmin.h
nlsaddr.o: $(INC)/stdio.h \
	$(INC)/ctype.h
nstoa.o:$(INC)/stdio.h \
	$(INC)/memory.h \
	$(INC)/malloc.h \
	$(INC)/ctype.h \
	nsaddr.h

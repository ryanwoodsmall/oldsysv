#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)script:mscript.mk	1.7"

OBJECTS = mscript.o do_line.o utilities.o callout.o mkscript.o
CURSES = -lcurses
LIB = $(CURSES)
INCLUDE =
CC = cc
CFLAGS =  -c $(INCLUDE)
FFLAG = 
LDFLAGS = $(FFLAG) -s
DEBUGFLAGS= 
BINDIR = ../../bin
INS = /etc/install

mscript : $(OBJECTS)
	$(CC) $(LDFLAGS) $(DEBUGFLAGS) -o mscript $(OBJECTS) $(LIB)
mscript.o : mscript.c  script.h
	$(CC) $(CFLAGS)  $(DEBUGFLAGS) mscript.c
do_line.o : do_line.c  script.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS) do_line.c
utilities.o : utilities.c  script.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS) utilities.c
callout.o : callout.c  script.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS) callout.c
mkscript.o : mkscript.c  script.h
	$(CC)  $(CFLAGS) $(DEBUGFLAGS) mkscript.c

install : mscript
	$(INS) -f $(BINDIR) mscript 

clobber : clean
	-rm -f mscript

clean:
	-rm -f $(OBJECTS)

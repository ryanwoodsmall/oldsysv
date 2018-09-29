#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)search:msearch.mk	1.6"
OBJECTS = msearch.o format.o syn.o
DEBUGFLAGS= 
BINDIR = ../../bin
LIBDIR = ../../lib
CC = cc
CFLAGS = -c
INS = /etc/install

msearch : $(OBJECTS)
	$(CC) -s $(DEBUGFLAGS) -o msearch $(OBJECTS)
msearch.o : msearch.c  msearch.h
	$(CC) $(CFLAGS)  $(DEBUGFLAGS) msearch.c
format.o : format.c  msearch.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS) format.c
syn.o : syn.c  msearch.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS) syn.c

install : msearch
	$(INS) -f $(BINDIR) msearch

clobber : clean
	-rm -f msearch

clean :
	-rm -f $(OBJECTS)

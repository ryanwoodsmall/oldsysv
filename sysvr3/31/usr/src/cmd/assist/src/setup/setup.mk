#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)setup:setup.mk	1.8"
OBJECTS = msetup.o intro.o term.o update.o
CC = cc
CFLAGS =  -c 
FFLAG =
LDFLAGS = $(FFLAG) -s
DEBUGFLAGS =
BINDIR = ../../bin
INS = /etc/install

msetup : $(OBJECTS)
	$(CC) $(LDFLAGS) -o msetup $(DEBUGFLAGS) $(OBJECTS)
msetup.o : msetup.c msetup.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS)  msetup.c
intro.o : intro.c msetup.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS)  intro.c
term.o : term.c msetup.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS)  term.c
update.o : update.c msetup.h
	$(CC) $(CFLAGS) $(DEBUGFLAGS)  update.c

install	: msetup
	$(INS) -f $(BINDIR) msetup 
	-rm -f $(BINDIR)/tsetup
	ln $(BINDIR)/msetup $(BINDIR)/tsetup

clobber : clean
	-rm -f tsetup
	-rm -f msetup

clean :
	-rm -f $(OBJECTS)


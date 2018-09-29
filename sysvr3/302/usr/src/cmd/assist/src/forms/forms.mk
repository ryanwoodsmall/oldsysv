#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)forms:forms.mk	1.11"

OBS = fill_out.o muse.o menu.o support.o read_in.o popup.o command.o update.o msupport.o valid.o expand.o
CURSES = -lcurses
LIBES = $(CURSES) -lm
INCLUDE =
CFLAGS=$(INCLUDE)
FFLAG=
LDFLAGS=$(FFLAG) -s
BINDIR = .
INS=/etc/install

all : mforms mecho
	:

mforms:	$(OBS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o mforms $(OBS) $(LIBES)

mecho : mecho.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o mecho mecho.c

msupport.o fill_out.o update.o command.o valid.o menu.o expand.o popup.o muse.o:	muse.h mmuse.h musedefs.h mmusedefs.h
support.o read_in.o:	muse.h  musedefs.h

install : mforms mecho
	$(INS) -f $(BINDIR) mforms 
	$(INS) -f $(BINDIR) mecho 

clobber : clean
	-rm -f mforms mecho

clean :
	-rm -f $(OBS)

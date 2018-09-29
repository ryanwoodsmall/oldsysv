#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfstop:rfstop.mk	1.7"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
LDFLAGS = -lns $(LDLIBS)
FRC =

all: rfstop

rfstop: rfstop.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/rfstop rfstop.c $(LDFLAGS)

install: all
	install -n $(INSDIR) $(TESTDIR)/rfstop

clean:
	rm -f rfstop.o

clobber: clean
	rm -f $(TESTDIR)/rfstop
FRC:

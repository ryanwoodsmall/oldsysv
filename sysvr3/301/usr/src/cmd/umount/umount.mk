#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)umount:umount.mk	1.3"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
FRC =

all: umount

umount: umount.c 
	$(CC) -I$(INCRT) $(CFLAGS) -o $(TESTDIR)/umount umount.c $(LDLIBS)

install: all
	install -n $(INSDIR) $(TESTDIR)/umount

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/umount
FRC:

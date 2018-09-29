#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fumount:fumount.mk	1.3"

#	fumount make file

ROOT=
INCDIR = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/bin
CFLAGS = -O -c -I$(INCDIR)
INS=/etc/install

all:	install clobber

fumount: fumount.o getnodes.o sndmes.o
	$(CC) -o fumount fumount.o getnodes.o sndmes.o
	chmod 4755 fumount

fumount.o: fumount.c $(INCDIR)/stdio.h $(INCDIR)/sys/types.h \
		$(INCDIR)/sys/idtab.h fumount.h
	$(CC) $(CFLAGS) fumount.c

getnodes.o: getnodes.c $(INCDIR)/nlist.h $(INCDIR)/stdio.h $(INCDIR)/fcntl.h \
		$(INCDIR)/sys/types.h $(INCDIR)/sys/sema.h \
		$(INCDIR)/sys/comm.h $(INCDIR)/sys/gdpstr.h \
		$(INCDIR)/sys/cirmgr.h $(INCDIR)/sys/idtab.h \
		$(INCDIR)/sys/mount.h $(INCDIR)/sys/adv.h fumount.h
	$(CC) $(CFLAGS) getnodes.c

sndmes.o: sndmes.c $(INCDIR)/stdio.h
	$(CC) $(CFLAGS) sndmes.c

debug.o: debug.c
	$(CC) $(CFLAGS) debug.c

install: fumount
	$(INS) -f $(INSDIR) -m 4755 -u root -g bin fumount

clean:
	-rm -f fumount.o getnodes.o sndmes.o

clobber: clean
	rm -f fumount

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fusage:fusage.mk	1.3"

#	fusage make file

ROOT=
INCDIR = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/bin
CFLAGS = -O -c -I$(INCDIR)
INS=/etc/install

all:	install clobber

fusage: fusage.o getnodes.o
	$(CC) -o fusage fusage.o getnodes.o $(LDLIBS)
	chmod 755 fusage

fusage.o: fusage.c $(INCDIR)/stdio.h $(INCDIR)/sys/types.h \
		$(INCDIR)/sys/stat.h $(INCDIR)/mnttab.h fumount.h \
		$(INCDIR)/sys/utsname.h 

getnodes.o: getnodes.c $(INCDIR)/nlist.h $(INCDIR)/stdio.h $(INCDIR)/fcntl.h \
		$(INCDIR)/sys/types.h $(INCDIR)/sys/sema.h \
		$(INCDIR)/sys/comm.h $(INCDIR)/sys/gdpstr.h \
		$(INCDIR)/sys/cirmgr.h $(INCDIR)/sys/idtab.h \
		$(INCDIR)/sys/mount.h $(INCDIR)/sys/adv.h fumount.h
	$(CC) $(CFLAGS) getnodes.c 

install: fusage
	$(INS) -f $(INSDIR) -m 755 -u root -g bin fusage

clean:
	-rm -f fusage.o

clobber: clean
	rm -f fusage

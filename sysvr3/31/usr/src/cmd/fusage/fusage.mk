#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fusage:fusage.mk	1.3.3.1"

#	fusage make file

ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/bin
CFLAGS = -O -c -I$(INC) 
LDFLAGS = -s
INS=/etc/install

all:	fusage

fusage: fusage.o getnodes.o
	$(CC) -o fusage $(LDFLAGS) fusage.o getnodes.o $(LDLIBS)
	chmod 755 fusage

fusage.o: fusage.c $(INC)/stdio.h $(INC)/sys/types.h \
		$(INC)/sys/stat.h $(INC)/mnttab.h fumount.h \
		$(INC)/sys/utsname.h $(INC)/sys/rfsys.h $(INC)/errno.h

getnodes.o: getnodes.c $(INC)/nlist.h $(INC)/stdio.h $(INC)/fcntl.h \
		$(INC)/sys/types.h $(INC)/sys/sema.h \
		$(INC)/sys/comm.h $(INC)/sys/gdpstr.h \
		$(INC)/sys/cirmgr.h $(INC)/sys/idtab.h \
		$(INC)/sys/mount.h $(INC)/sys/adv.h fumount.h
	$(CC) $(CFLAGS) getnodes.c 

install: fusage
	$(INS) -f $(INSDIR) -m 755 -u root -g bin fusage

clean:
	-rm -f fusage.o

clobber: clean
	rm -f fusage

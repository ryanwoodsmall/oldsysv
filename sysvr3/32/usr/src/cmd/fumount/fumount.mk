#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fumount:fumount.mk	1.4.3.1"

#	fumount make file

ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/bin
CFLAGS = -O -c -I$(INC)
LDFLAGS = -s
INS=/etc/install

all:	fumount

fumount: fumount.o getnodes.o sndmes.o
	$(CC) -o fumount $(LDFLAGS) fumount.o getnodes.o sndmes.o
	chmod 755 fumount

fumount.o: fumount.c $(INC)/stdio.h $(INC)/sys/types.h \
		$(INC)/sys/idtab.h fumount.h
	$(CC) $(CFLAGS) fumount.c

getnodes.o: getnodes.c $(INC)/nlist.h $(INC)/stdio.h $(INC)/fcntl.h \
		$(INC)/sys/types.h $(INC)/sys/sema.h \
		$(INC)/sys/comm.h $(INC)/sys/gdpstr.h \
		$(INC)/sys/cirmgr.h $(INC)/sys/idtab.h \
		$(INC)/sys/mount.h $(INC)/sys/adv.h fumount.h
	$(CC) $(CFLAGS) getnodes.c

sndmes.o: sndmes.c $(INC)/stdio.h
	$(CC) $(CFLAGS) sndmes.c

debug.o: debug.c
	$(CC) $(CFLAGS) debug.c

install: fumount
	$(INS) -f $(INSDIR) -m 755 -u root -g bin fumount

clean:
	-rm -f fumount.o getnodes.o sndmes.o

clobber: clean
	rm -f fumount

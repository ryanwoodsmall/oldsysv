#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfsetup:rfsetup.mk	1.9"
ROOT =
NSLIB = -lns
LIB = $(NSLIB) -lnsl_s -lcrypt
INSDIR = $(ROOT)/usr/net/servers/rfs
INC = $(ROOT)/usr/include
LOG=-DLOGGING
DEBUG=
LDFLAGS=-s
CFLAGS=-O $(DEBUG) $(LOG)

all:	rfsetup

rfsetup: rfsetup.o
	$(CC) $(CFLAGS) rfsetup.o $(LDFLAGS) $(LIB) -o rfsetup

install: all
	-@if [ ! -d "$(INSDIR)" ] ; \
	then \
		(	\
		mkdir $(ROOT)/usr/net $(ROOT)/usr/net/servers $(INSDIR) >/dev/null 2>&1 ; \
		exit 0;	\
		) ;	\
	fi ;
	cp rfsetup $(INSDIR)

uninstall:
	(cd $(INSDIR); -rm -f rfsetup )

clean:
	-rm -f *.o

clobber: clean
	-rm -f rfsetup

#### dependencies now follow

rfsetup.o: \
	$(INC)/sys/stropts.h \
	$(INC)/sys/cirmgr.h \
	$(INC)/sys/rfsys.h \
	$(INC)/sys/hetero.h \
	$(INC)/pn.h \
	$(INC)/nserve.h \
	$(INC)/errno.h \
	$(INC)/string.h \
	$(INC)/stdio.h \
	$(INC)/fcntl.h

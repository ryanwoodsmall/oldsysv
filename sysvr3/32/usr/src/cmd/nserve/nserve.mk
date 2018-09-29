#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nserve:nserve.mk	1.11.4.1"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/nserve
INC = $(ROOT)/usr/include
INS = :
CFLAGS=-O -s $(DEBUG) $(LOG) $(PROFILE)
CC = cc
NSLIB = -lns
NSL = nsl_s
LIB = $(NSLIB) -l$(NSL) -lcrypt
LLIB = $(ROOT)/usr/src/lib/libns/llib-lns.ln
LOG=-DLOGGING -DLOGMALLOC
PROFILE=
DEBUG=
EXECS=nserve
SOURCE=nserve.c nsrec.c nsfunc.c nsdb.c
OBJECTS=nserve.o nsrec.o nsfunc.o nsdb.o
FRC =

all:	$(EXECS)
nserve: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIB) -o $(TESTDIR)/nserve $(LDLIBS)
debug:
	$(MAKE) -f nserve.mk DEBUG="-g -DLOGGING -DLOGMALLOC" all
dashg:
	$(MAKE) -f nserve.mk NSL=nsl NSLIB=-lnsdb DEBUG="-g -DLOGGING -DLOGMALLOC" all
lint:
	lint -pua $(SOURCE) $(LLIB)

install: all
	-@if [ ! -d "$(INSDIR)" ] ; \
	then \
		mkdir $(INSDIR) ; \
	fi ;
	/etc/install -f $(INSDIR) $(EXECS)
	$(CH)chmod 550 $(INSDIR)/$(EXECS)
uninstall:
	(cd $(INSDIR); rm -f $(EXECS))

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(EXECS)
FRC: 

#### dependencies now follow

nserve.o: nsdb.h nslog.h $(INC)/nsaddr.h stdns.h $(INC)/nserve.h nsports.h
nsrec.o: nsdb.h nslog.h $(INC)/nsaddr.h stdns.h $(INC)/nserve.h nsports.h
nsdb.o: nsdb.h stdns.h nslog.h
nsfunc.o: nsdb.h stdns.h $(INC)/nserve.h nslog.h

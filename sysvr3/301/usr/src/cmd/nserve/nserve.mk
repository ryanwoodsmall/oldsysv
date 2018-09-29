#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nserve:nserve.mk	1.11"
ROOT =
CC = cc
NSLIB = -lns
NSL = nsl_s
LIB = $(NSLIB) -l$(NSL) -lcrypt
LLIB = $(ROOT)/usr/src/lib/libns/llib-lns.ln
INSDIR = $(ROOT)/usr/nserve
LOG=-DLOGGING -DLOGMALLOC
PROFILE=
DEBUG=
INCDIR=$(ROOT)/usr/include
CFLAGS=-O $(DEBUG) $(LOG) $(PROFILE)
EXECS=nserve
SOURCE=nserve.c nsrec.c nsfunc.c nsdb.c
OBJECTS=nserve.o nsrec.o nsfunc.o nsdb.o

all:	$(EXECS)
nserve: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIB) -o nserve $(LDLIBS)
debug:
	make -f nserve.mk DEBUG="-g -DLOGGING -DLOGMALLOC" all
dashg:
	make -f nserve.mk NSL=nsl NSLIB=-lnsdb DEBUG="-g -DLOGGING -DLOGMALLOC" all
lint:
	lint -pua $(SOURCE) $(LLIB)

install: all
	-@if [ ! -d "$(INSDIR)" ] ; \
	then \
		mkdir $(INSDIR) ; \
	fi ;
	cp $(EXECS) $(INSDIR)
uninstall:
	(cd $(INSDIR); -rm -f $(EXECS))

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(EXECS)

#### dependencies now follow

nserve.o: nsdb.h nslog.h $(INCDIR)/nsaddr.h stdns.h $(INCDIR)/nserve.h nsports.h
nsrec.o: nsdb.h nslog.h $(INCDIR)/nsaddr.h stdns.h $(INCDIR)/nserve.h nsports.h
nsdb.o: nsdb.h stdns.h nslog.h
nsfunc.o: nsdb.h stdns.h $(INCDIR)/nserve.h nslog.h

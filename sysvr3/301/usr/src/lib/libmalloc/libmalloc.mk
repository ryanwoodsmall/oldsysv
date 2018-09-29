#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libmalloc:libmalloc.mk	1.18"
#
# makefile for libmalloc
#

.SUFFIXES: .p
ROOT=
AR=ar
ARFLAGS=r
CC=cc
CFLAGS=-O
CP=cp
ENV=
INC= $(ROOT)/usr/include
LIB=$(ROOT)/lib
USRLIB= $(ROOT)/usr/lib
LIBP= $(ROOT)/usr/lib/libp
LINT=lint
LINTFLAGS=-p
MV=mv
OBJECTS=malloc.o
POBJECTS=malloc.p
RM=rm
#variables used to build lint library in 5.0 compatible mode
LINT1=/usr/lib/lint1
LINT2=/usr/lib/lint2
TMPDIR=/usr/tmp
FRC=

all: noprof prof llib-lmalloc.l

noprof: $(OBJECTS)
	$(RM) -rf objects
	mkdir objects
	$(CP) $(OBJECTS) objects
	cd objects; $(AR) $(ARFLAGS) libmalloc.a *

prof: $(POBJECTS)
	$(RM) -rf pobjects
	mkdir pobjects
	for i in $(POBJECTS);\
	do\
		$(CP) $$i pobjects/`basename $$i .p`.o;\
	done
	cd pobjects; $(AR) $(ARFLAGS) libmalloc.a *

malloc.o malloc.p: $(INC)/malloc.h mallint.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c    && $(MV) $(*F).o $*.p
	$(CC) $(CFLAGS) -c $*.c

#*****************************************************************************
#           code as in lint.sh to implement -c option
#		on pre SVR2 systems
#*****************************************************************************
llib-lmalloc.l:  llib-lmall.c $(LINT1) $(FRC) $(LINT2)
	cp llib-lmall.c llib-lmalloc.c
	$(CC) -E -C -Dlint llib-lmalloc.c | \
		$(LINT1) -H$(TMPDIR)/hlint >llib-lmalloc.ln 
	$(LINT2) -Tllib-lmalloc.ln -H$(TMPDIR)/hlint
	rm -f llib-lmalloc.c

install: all
	if [ ! -d $(LIBP) ];\
	then\
		mkdir $(LIBP);\
	fi
	$(CP) objects/libmalloc.a $(USRLIB)/libmalloc.a
	$(CP) pobjects/libmalloc.a $(LIBP)/libmalloc.a
	if pdp11; then \
		$(USRLIB)/libmalloc.a; \
		$(LIBP)/libmalloc.a; \
	fi
	$(CP) llib-lmalloc.l $(USRLIB)/llib-lmalloc.l

clean: 
	$(RM) -fr pobjects objects

clobber: clean
	$(RM) -f $(POBJECTS) $(OBJECTS)

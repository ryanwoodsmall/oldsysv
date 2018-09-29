#	@(#)libmalloc.mk	1.7
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
LIB= $(ROOT)/usr/lib
PROFDIR= $(ROOT)/lib/libp
LINT=lint
LINTFLAGS=-p
MV=mv
OBJECTS=malloc.o
POBJECTS=malloc.p
RM=rm
SGS=
STRIP=strip
SPFLAGS=-r

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

llib-lmalloc.l:  llib-lmall.c
	cp llib-lmall.c llib-lmalloc.c
	lint -c llib-lmalloc.c
	rm llib-lmalloc.c

install: all
	if [ ! -d $(PROFDIR) ];\
	then\
		mkdir $(PROFDIR);\
	fi
	$(CP) objects/libmalloc.a $(LIB)/libmalloc.a
	$(CP) pobjects/libmalloc.a $(PROFDIR)/libmalloc.a
	if pdp11; then \
		$(STRIP) $(LIB)/libmalloc.a; \
		$(STRIP) $(PROFDIR)/libmalloc.a; \
	else \
		$(STRIP) $(SPFLAGS) $(LIB)/libmalloc.a; \
		$(STRIP) $(SPFLAGS) $(PROFDIR)/libmalloc.a; \
		$(AR) ts $(LIB)/libmalloc.a; \
		$(AR) ts $(PROFDIR)/libmalloc.a; \
	fi
	$(CP) llib-lmalloc.l $(ROOT)/usr/lib/llib-lmalloc.l

clean: 
	$(RM) -fr pobjects objects

clobber: clean
	$(RM) -f $(POBJECTS) $(OBJECTS)

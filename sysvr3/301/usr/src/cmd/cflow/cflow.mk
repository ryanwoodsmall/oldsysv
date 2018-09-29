#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cflow:cflow.mk	1.12"
FLG=
TESTDIR = .
CFLAGS =-O -DFLEXNAMES -DUNIX5
FILES = Makefile README cflow.sh dag.c lpfx.c nmf.c flip.c
ALL = dag lpfx nmf flip
LINT = .
MIP = .
BIN = $(ROOT)/usr/bin
LIB = $(ROOT)/usr/lib
LDFLAGS = -s

all:	chk_pdp $(ALL)

chk_pdp:
	if pdp11 && [ "$(FLG)" != "x" ]; then \
		$(MAKE) -$(MAKEFLAGS) FLG=x ALL="$(ALL)" LINT="$(LINT)" \
			MIP="$(MIP)" CFLAGS="$(CFLAGS) -UFLEXNAME" \
			-f cflow.mk all; \
	fi

dag:	dag.c
	$(CC) $(CFLAGS) $(LDFLAGS) -i dag.c -o $(TESTDIR)/dag

lpfx:	lpfx.c $(LINT)/lerror.h $(LINT)/lmanifest $(LINT)/lpass2.h \
		$(MIP)/manifest
	$(CC) $(CFLAGS) -I$(LINT) -I$(MIP) $(LDFLAGS) -i lpfx.c -o $(TESTDIR)/lpfx -lmalloc

nmf:	nmf.c
	$(CC) $(CFLAGS) nmf.c $(LDFLAGS) -o $(TESTDIR)/nmf

flip:	flip.c
	$(CC) $(CFLAGS) flip.c $(LDFLAGS) -o $(TESTDIR)/flip

install:	chk_pdp $(ALL)
	cp cflow.sh $(BIN)/cflow
	cp dag $(LIB)/dag
	cp lpfx $(LIB)/lpfx
	cp nmf $(LIB)/nmf
	cp flip $(LIB)/flip
	$(CH) chgrp bin $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip
	$(CH) chown bin $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip
	$(CH) chmod 755 $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip

clean:
	-rm -f *.o a.out make.out core

clobber:	clean
	-rm -f $(ALL)

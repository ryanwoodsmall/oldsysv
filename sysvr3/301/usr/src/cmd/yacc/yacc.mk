#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)yacc:yacc.mk	1.5"
ROOT=
INC=$(ROOT)/usr/include
UBIN=$(ROOT)/usr/bin
ULIB=$(ROOT)/usr/lib
CC=cc
CFLAGS=-O -I$(INC)
TESTDIR = .
FRC =
INS = install
INSDIR =
OFILES = y1.o y2.o y3.o y4.o

all: yacc

yacc:	$(OFILES)
	$(CC) $(LDFLAGS) $(IFLAG) -s -o $(TESTDIR)/yacc $(OFILES)

$(OFILES): dextern files $(FRC)
	   $(CC) -c $(CFLAGS) y1.c y2.c y3.c y4.c

test:
	   rtest $(TESTDIR)/yacc

install:   all
	   $(INS) -n $(UBIN) $(TESTDIR)/yacc $(INSDIR)
	   $(INS) -n $(ULIB) $(TESTDIR)/yaccpar

clean:
	   -rm -f $(OFILES)

clobber:   clean
	   -rm -f $(TESTDIR)/yacc

FRC:

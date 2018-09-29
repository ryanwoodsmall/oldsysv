#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lex:lex.mk	1.6"
ROOT=
INC=$(ROOT)/usr/include
TESTDIR = .
UBIN=$(ROOT)/usr/bin
INS = install
CFLAGS = -O -I$(INC)
FRC =
INSDIR =
FILES = main.o sub1.o sub2.o header.o

all: lex

lex:	$(FILES) y.tab.o
	$(CC) $(LDFLAGS) $(IFLAG) -s $(FILES) y.tab.o -ly -o $(TESTDIR)/lex

$(FILES): ldefs.c $(FRC)
main.o:	  once.c $(FRC)
y.tab.c:  parser.y $(FRC)
	  $(YACC) parser.y

test:
	  rtest $(TESTDIR)/lex

install: all
	 $(INS) -n $(UBIN) $(TESTDIR)/lex $(INSDIR)

clean:
	 -rm -f *.o y.tab.c

clobber: clean
	 -rm -f $(TESTDIR)/lex

lint:	main.c sub1.c sub2.c header.c y.tab.c once.c ldefs.c
	lint -p main.c sub1.c sub2.c header.c y.tab.c once.c ldefs.c

FRC:

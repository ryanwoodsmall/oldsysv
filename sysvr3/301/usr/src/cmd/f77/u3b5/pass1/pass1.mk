#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)f77:u3b5/pass1/pass1.mk	1.5.3.2"
#	3B5 or 3B15 FORTRAN COMPILER PASS1 MAKEFILE

DEFS = -DHERE=X3B -DTARGET=X3B -DFAMILY=PCC
DIRCOM = ../../common/pass1
DIRMAC = ../../u3b/pass1
DIRBLD = .
CFLAGS = -O $(DEFS) -I$(DIRBLD) -I$(DIRCOM) -I$(DIRMAC) -I. $(FFLAG)
LDFLAGS =
CC = cc
SIZE = size
STRIP = strip
INSTALL = install

OBJECTS = main.o init.o gram.o lex.o proc.o equiv.o data.o \
	  expr.o exec.o intr.o io.o misc.o error.o put.o \
	  putpcc.o 3b.o 3bx.o
INCLUDES = $(DIRCOM)/defs $(DIRCOM)/defines $(DIRMAC)/machdefs $(DIRCOM)/ftypes
GRAMMAR = $(DIRCOM)/gram.head $(DIRCOM)/gram.dcl $(DIRCOM)/gram.expr \
	  $(DIRCOM)/gram.exec $(DIRCOM)/gram.io

all:		f77 f77pass1

f77:		driver.o 3bx.o
		$(CC) $(CFLAGS) $(LDFLAGS) driver.o 3bx.o -o f77
		@$(SIZE) f77

f77pass1:	$(OBJECTS)
		$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o f77pass1
		@$(SIZE) f77pass1

driver.o:	$(DIRCOM)/driver.c $(DIRBLD)/drivedefs $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/driver.c

main.o:		$(DIRCOM)/main.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/main.c

init.o:		$(DIRCOM)/init.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/init.c

gram.o:		gram.c $(INCLUDES)
		$(CC) -c $(CFLAGS) gram.c

lex.o:		$(DIRCOM)/lex.c $(INCLUDES) tokdefs
		$(CC) -c $(CFLAGS) $(DIRCOM)/lex.c

proc.o:		$(DIRCOM)/proc.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/proc.c

equiv.o:	$(DIRCOM)/equiv.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/equiv.c

data.o:		$(DIRCOM)/data.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/data.c

expr.o:		$(DIRCOM)/expr.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/expr.c

exec.o:		$(DIRCOM)/exec.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/exec.c

intr.o:		$(DIRCOM)/intr.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/intr.c

io.o:		$(DIRCOM)/io.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/io.c

misc.o:		$(DIRCOM)/misc.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/misc.c

error.o:	$(DIRCOM)/error.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRCOM)/error.c

put.o:		$(DIRCOM)/put.c $(INCLUDES) $(DIRCOM)/pccdefs
		$(CC) -c $(CFLAGS) $(DIRCOM)/put.c

putpcc.o:	$(DIRCOM)/putpcc.c $(INCLUDES) $(DIRCOM)/pccdefs
		$(CC) -c $(CFLAGS) $(DIRCOM)/putpcc.c

3b.o:		$(DIRMAC)/3b.c $(INCLUDES) $(DIRCOM)/pccdefs
		$(CC) -c $(CFLAGS) $(DIRMAC)/3b.c

3bx.o:		$(DIRMAC)/3bx.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRMAC)/3bx.c

gram.c:		$(GRAMMAR) tokdefs
		sed < tokdefs "s/#define/%token/" > gram.tmp
		for i in $(GRAMMAR); \
		do \
			sed -e "1,/%}/d" < $$i >> gram.tmp; \
		done
		cat idents gram.tmp > gram.in
		$(YACC) $(YFLAGS) gram.in
		@echo "expect 4 shift/reduce"
		mv y.tab.c gram.c
		rm gram.in gram.tmp

tokdefs:	$(DIRCOM)/tokens
		egrep -n "%{|%}|^#ident" $(DIRCOM)/tokens $(GRAMMAR) | \
		cut -d: -f2-30 | \
		egrep  "^1:|^2:|^3:" | cut -d: -f2-30 > idents
		egrep -v "%{|%}|^#ident" < $(DIRCOM)/tokens | grep -n "^[^#]" | \
		sed "s/\([^:]*\):\(.*\)/#define \2 \1/" > tokdefs

install:	all
	$(STRIP) f77
	$(STRIP) f77pass1
	$(INSTALL) -f $(ROOT)/usr/bin f77
	$(INSTALL) -f $(ROOT)/usr/lib f77pass1

clean:
	-rm -f gram.c *.o tokdefs idents

clobber:	clean
	-rm -f f77 f77pass1

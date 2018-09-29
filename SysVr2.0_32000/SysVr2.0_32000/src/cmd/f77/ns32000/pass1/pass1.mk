#	@(#)pass1.mk	1.2
#	NSC FORTRAN COMPILER PASS1 MAKEFILE

DEFS = -DHERE=NSC -DTARGET=NSC -DFAMILY=PCC
DIRCOM = ../../common/pass1
DIRMAC = .
CFLAGS = -O $(DEFS) -I$(DIRCOM) -I$(DIRMAC) -I.
LDFLAGS =
CC = cc
SIZE = size
STRIP = strip
INSTALL = install

OBJECTS = main.o init.o gram.o lex.o proc.o equiv.o data.o \
	  expr.o exec.o intr.o io.o misc.o error.o put.o \
	  putpcc.o nsc.o nscx.o
INCLUDES = $(DIRCOM)/defs $(DIRCOM)/defines $(DIRMAC)/machdefs $(DIRCOM)/ftypes
GRAMMAR = $(DIRCOM)/gram.head $(DIRCOM)/gram.dcl $(DIRCOM)/gram.expr \
	  $(DIRCOM)/gram.exec $(DIRCOM)/gram.io

all:		f77 f77pass1

f77:		driver.o nscx.o
		$(CC) $(LDFLAGS) driver.o nscx.o -o f77
		@$(SIZE) f77

f77pass1:	$(OBJECTS)
		$(CC) $(LDFLAGS) $(OBJECTS) -o f77pass1
		@$(SIZE) f77pass1

driver.o:	$(DIRCOM)/driver.c $(DIRMAC)/drivedefs $(INCLUDES)
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

nsc.o:		$(DIRMAC)/nsc.c $(INCLUDES) $(DIRCOM)/pccdefs
		$(CC) -c $(CFLAGS) $(DIRMAC)/nsc.c

nscx.o:		$(DIRMAC)/nscx.c $(INCLUDES)
		$(CC) -c $(CFLAGS) $(DIRMAC)/nscx.c

gram.c:		$(GRAMMAR) tokdefs
		( sed < tokdefs "s/#define/%token/" ;\
		cat $(GRAMMAR) ) > gram.in
		$(YACC) $(YFLAGS) gram.in
		@echo "expect 4 shift/reduce"
		mv y.tab.c gram.c
		rm gram.in

tokdefs:	$(DIRCOM)/tokens
		grep -n "^[^#]" < $(DIRCOM)/tokens | \
		sed "s/\([^:]*\):\(.*\)/#define \2 \1/" > tokdefs

install:
	$(STRIP) f77
	$(STRIP) f77pass1
	$(INSTALL) -f $(ROOT)/usr/bin f77
	$(INSTALL) -f $(ROOT)/usr/lib f77pass1

clean:
	-rm -f gram.c *.o tokdefs

clobber:	clean
	-rm -f f77 f77pass1

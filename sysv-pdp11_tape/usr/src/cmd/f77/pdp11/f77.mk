#	@(#)f77.mk	1.2
#	3.0 SID #	1.3
#	Makefile for a Fortran 77 compiler
#	running on the PDP11, generating code for the PDP11,
#	using the second pass of the Ritchie C compiler as code generator.

CFL = -DHERE=PDP11 -DTARGET=PDP11 -DFAMILY=DMR -DNEWDMR

CFLAGS = -O $(CFL)

OBJECTS = fltused.o main.o init.o gram.o lex.o proc.o equiv.o data.o \
	  expr.o exec.o intr.o io.o misc.o error.o put.o \
	  putdmr.o pdp11.o pdp11x.o

compiler : f77 f77pass1
	touch compiler

f77 : driver.o pdp11x.o
	$(CC) $(LDFLAGS) $(IFLAG) -n driver.o pdp11x.o -o f77
	@size f77

f77pass1 : $(OBJECTS)
	@echo LOAD
	@$(CC) $(LDFLAGS) $(IFLAG) $(OBJECTS) -o f77pass1
	@size f77pass1

gram.c:	gram.head gram.dcl gram.expr gram.exec gram.io tokdefs
	( sed <tokdefs "s/#define/%token/" ;\
		cat gram.head gram.dcl gram.expr gram.exec gram.io ) >gram.in
	$(YACC) $(YFLAGS) gram.in
	@echo "(expect 4 shift/reduce)"
	mv y.tab.c gram.c
	rm gram.in

tokdefs: tokens
	grep -n "^[^#]" <tokens | sed "s/\([^:]*\):\(.*\)/#define \2 \1/" >tokdefs
lex.o : tokdefs
driver.o $(OBJECTS)  : defs defines machdefs ftypes
driver.o : drivedefs
io.o : fio.h

machdefs : pdp11defs
	cp pdp11defs machdefs

put.o putdmr.o pdp11.o : dmrdefs

install : /usr/bin/f77 /usr/lib/f77pass1

/usr/bin/f77 : f77
	@size f77 /usr/bin/f77
	cp f77 /usr/bin/f77

/usr/lib/f77pass1 : f77pass1
	@size f77pass1 /usr/lib/f77pass1
	cp f77pass1 /usr/lib/f77pass1

clean:
	-rm -f gram.c *.o machdefs tokdefs compiler

clobber: clean
	-rm -f f77 f77pass1

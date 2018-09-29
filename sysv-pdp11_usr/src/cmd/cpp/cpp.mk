#	@(#)cpp.mk	1.6
INS= /etc/install
TESTDIR=.
INSDIR=/lib
CFLAGS=-O

cpp :	cpp.o cpy.o rodata.o
	$(CC) $(LDFLAGS) -o $(TESTDIR)/cpp cpp.o rodata.o cpy.o  

cpy.c : cpy.y
	$(YACC) cpy.y
	-chmod +x ./:yyfix
	./:yyfix >rodata.c
	mv y.tab.c cpy.c

cpy.o :	cpy.c yylex.c

cpp.o : cpp.c
	if vax;		then $(CC) $(CFLAGS) -Dunix -Dvax -c cpp.c; \
	elif pdp11;	then $(CC) $(CFLAGS) -Dunix -Dpdp11 -c cpp.c; \
	elif u3b;	then $(CC) $(CFLAGS) -Dunix -Du3b -c cpp.c; \
	fi

rodata.o : cpy.c
	$(CC) $(CFLAGS) -S rodata.c
	-chmod +x ./:rofix
	./:rofix rodata.s
	$(CC) $(CFLAGS) -c rodata.s

clean :
	rm -f *.s *.o

clobber: clean
	rm -f cpy.c rodata.c $(TESTDIR)/cpp

install :  cpp
	$(INS) cpp $(INSDIR)/cpp

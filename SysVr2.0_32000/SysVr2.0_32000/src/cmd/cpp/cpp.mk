#	@(#)cpp.mk	1.9
INS	= /etc/install
TESTDIR	= .
INSDIR	= /lib
CFLAGS	= -O -DFLEXNAMES
LDFLAGS	= -s

cpp :	cpp.o cpy.o rodata.o
	$(CC) $(LDFLAGS) -o $(TESTDIR)/cpp cpp.o rodata.o cpy.o  

cpy.c : cpy.y
	$(YACC) cpy.y
	-if pdp11 || vax || ns32000; \
	then \
		chmod +x ./:yyfix; \
		./:yyfix >rodata.c; \
	else \
		>rodata.c; \
	fi
	mv y.tab.c cpy.c

cpy.o :	cpy.c yylex.c
	-if pdp11; then	$(CC) $(CFLAGS) -UFLEXNAME -c cpy.c; \
	else		$(CC) $(CFLAGS)            -c cpy.c;	fi

cpp.o : cpp.c
	if pdp11;  then $(CC) $(CFLAGS) -Dunix -Dpdp11 -UFLEXNAME -c cpp.c; \
	elif vax;  then $(CC) $(CFLAGS) -Dunix -Dvax              -c cpp.c; \
	elif ns32000;  then $(CC) $(CFLAGS) -Dunix -Dns32000      -c cpp.c; \
	elif u3b;  then $(CC) $(CFLAGS) -Dunix -Du3b              -c cpp.c; \
	elif u3b5; then $(CC) $(CFLAGS) -Dunix -Du3b5             -c cpp.c; \
	else       echo 'Unknown machine???'; exit 2; \
	fi

rodata.o : cpy.c
	$(CC) $(CFLAGS) -S rodata.c
	-if pdp11 || vax || ns32000; \
	then \
		chmod +x ./:rofix; \
		./:rofix rodata.s; \
	fi
	$(CC) $(CFLAGS) -c rodata.s

clean :
	rm -f *.s *.o

clobber: clean
	rm -f cpy.c rodata.c $(TESTDIR)/cpp

install : cpp
	$(INS) -f $(INSDIR) cpp

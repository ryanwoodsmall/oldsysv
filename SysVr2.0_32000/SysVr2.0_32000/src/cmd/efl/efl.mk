#	@(#)efl.mk	1.8
INS = install 
INSDIR = $(ROOT)/usr/bin
CFLAGS = -O
FFLAG =
LDFLAGS = -i
IFLAG = -i
STRIP=strip
FILES =  Makefile efl.1 defs tokens main.c init.c tailor.c\
		gram.head gram.dcl gram.expr gram.exec\
		lex.l fixuplex symtab.c\
		dcl.c addr.c struct.c field.c misc.c alloc.c mk.c\
		exec.c blklab.c simple.c print.c temp.c io.c error.c\
		pass2.c icfile.c free.c dclgen.c namgen.c
OFILES = main.o init.o tailor.o gram.o lex.o symtab.o\
	dcl.o addr.o struct.o field.o blklab.o\
	mk.o misc.o alloc.o print.o simple.o exec.o temp.o io.o error.o\
	pass2.o icfile.o free.o dclgen.o namgen.o

all:	itest efl

itest:
	@if [ x$(IFLAG) != x-i ] ; then echo NO ID SPACE ; exit 1 ; else exit 0 ; fi
efl:	$(OFILES)
	$(CC) -o efl $(FFLAG) $(CFLAGS) $(LDFLAGS) $(OFILES)

$(OFILES):	defs
lex.o init.o:	tokdefs

gram.c:	gram.head gram.dcl gram.expr gram.exec tokdefs
	set +e; if vax || ns32000 || u3b ;\
	then	(sed <tokdefs "s/#define/%token/" ;\
		cat gram.head gram.dcl gram.expr gram.exec) >gram.in;\
		$(YACC) $(YFLAGS) gram.in;  mv y.tab.c gram.c;  rm gram.in;\
	else	cp gram.x gram.c;\
	fi

lex.c:	fixuplex lex.l
	lex lex.l
	sh ./fixuplex
	mv lex.yy.c lex.c

tokdefs: tokens
	grep -n "^[^#]" <tokens | sed "s/\([^:]*\):\(.*\)/#define \2 \1/" >tokdefs

.c.o:
	$(CC) -c $(CFLAGS) $< 2>&1 | sed "/arning/d"

clean:
	rm -f *.o efl[cde].* gram.c lex.c tokdefs

clobber:	clean
	rm -f efl

install :	all
	$(STRIP) efl
	$(INS) -f $(INSDIR) efl

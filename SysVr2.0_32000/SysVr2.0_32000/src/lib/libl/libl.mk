#	@(#)libl.mk	1.2
ROOT=
ULIBDIR=$(ROOT)/usr/lib

CC=cc
CFLAGS=
LORDER=lorder
LIBOBJS=allprint.o main.o reject.o yyless.o yywrap.o

all:     $(ULIBDIR)/libl.a

$(ULIBDIR)/libl.a: $(LIBOBJS)
	ar rv tmplib.a `$(LORDER) *.o | tsort`;

install:  ncform nrform all
	cp tmplib.a $(ULIBDIR)/libl.a;

clean:
	-rm -f *.o

clobber:	clean
	-rm -f tmplib.a 

allprint.o:	lib/allprint.c
	$(CC) -c $(CFLAGS) lib/allprint.c
main.o:		lib/main.c
	$(CC) -c $(CFLAGS) lib/main.c
reject.o:		lib/reject.c
	$(CC) -c $(CFLAGS) lib/reject.c
yyless.o:		lib/yyless.c
	$(CC) -c $(CFLAGS) lib/yyless.c
yywrap.o:		lib/yywrap.c
	$(CC) -c $(CFLAGS) lib/yywrap.c

ncform:	lib/ncform
	cp lib/ncform $(ULIBDIR)/lex/ncform;

nrform:	lib/nrform
	cp lib/nrform $(ULIBDIR)/lex/nrform;

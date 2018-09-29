#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libl:libl.mk	1.7"
ROOT=
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib

CC=cc
CFLAGS=
LORDER=lorder
LIBOBJS=allprint.o main.o reject.o yyless.o yywrap.o

all:     $(USRLIB)/libl.a

$(USRLIB)/libl.a: $(LIBOBJS)
	ar rv tmplib.a `$(LORDER) *.o | tsort`;

install:  ncform nrform all
	cp tmplib.a $(USRLIB)/libl.a;

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
	if [ ! -d $(USRLIB)/lex ];\
		then mkdir $(USRLIB)/lex;\
	fi
	-rm -f $(USRLIB)/lex/ncform;
	cp lib/ncform $(USRLIB)/lex/ncform;

nrform:	lib/nrform
	-rm -f $(USRLIB)/lex/nrform;
	cp lib/nrform $(USRLIB)/lex/nrform;

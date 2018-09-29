#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)shl:shl.mk	1.4"

ROOT=
INSDIR=$(ROOT)/usr/bin
CFLAGS= -O
LDFLAGS= -s
LPDEST= laser

OBJECTS= defs.o yacc.o newmain.o mpx.o layer.o misc.o stat.o signal.o \
	 system.o aux.o

shlayers:	$(OBJECTS)
		$(CC) $(CFLAGS) $(LDFLAGS) -o shl $(OBJECTS) -ll

print:
		dupsendto $(LPDEST) shl.mk defs.h defs.c yacc.y lex.l \
				 newmain.c mpx.c layer.c misc.c stat.c  \
				 signal.c aux.c system.c
yacc.o	:	yacc.c lex.c

yacc.c	:	yacc.y

lex.c	:	lex.l

aux.o defs.o newmain.o mpx.o layer.o misc.o stat.o signal.o system.o	: defs.h


install:	shlayers
		install -o -f $(INSDIR) -m 4755 -u root -g sys shl

clobber:	clean
		rm -f shl

clean:		
		rm -f lex.c yacc.c *.o

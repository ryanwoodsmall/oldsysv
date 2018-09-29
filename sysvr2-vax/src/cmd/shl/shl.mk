#	@(#)shl.mk	1.2

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
		/etc/install -o -f /usr/bin shl
		chown root /usr/bin/shl
		chgrp sys /usr/bin/shl
		chmod 4755 /usr/bin/shl

clobber:	clean
		rm -f shl

clean:		
		rm -f lex.c yacc.c *.o

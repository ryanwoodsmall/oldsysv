#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mkboot-3b2:mkboot.mk	1.6"
INC = $(ROOT)/usr/include
SOURCES = $(INC)/sys/boothdr.h mkboot.h xmkboot.l mkboot.y main.c util.c
OBJECTS = mkboot.o main.o util.o
LEXLIB = -ll
CFLAGS = -s -O -I$(INC) -Uvax -Updp11 -Du3b5
#CFLAGS = -g -N -I$(INC) -DYYDEBUG=1 -DLEXDEBUG=1
PR = xcl -x -2 -pmini

mkboot:	$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LEXLIB) -o mkboot

install:mkboot
	install -f $(ROOT)/etc mkboot

main.o:	mkboot.h $(INC)/sys/boothdr.h
util.o:	mkboot.h $(INC)/sys/boothdr.h
mkboot.o:lex.yy.c mkboot.h $(INC)/sys/boothdr.h

lex.yy.c:xmkboot.l
	$(LEX) xmkboot.l


clobber:clean
	rm -f mkboot

clean:
	rm -f lex.yy.c *.o tags cscope.out

print:	mkboot.mk $(SOURCES)
	$(PR) $?
	touch print

.PRECIOUS: print

lint:	main.c util.c mkboot.y lex.yy.c
	$(YACC) mkboot.y
	lint -b $(CFLAGS) main.c util.c y.tab.c
	rm -f y.tab.c

tags:	$(SOURCES)
	ctags $(SOURCES)

cscope.out:$(SOURCES)
	@-if [ "$(RUN)" ]; \
	then \
		cscope -I$(INC) $(SOURCES) \
	else \
		cscope -I$(INC) $(SOURCES) </dev/null \
	fi

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cxref:cxref.mk	1.14"
FLG=
OWNER=$(ROOT)/usr/lib
CC=cc
# comp and other contain files shared with C preprocessor, lint and comp
M=./comp
L=./other
CPP=./other
INCS=-I. -I$M -I$L
DEFS=-DCXREF -DFLEXNAMES
LINTF = -p
LINT = lint
OPRL = oprl
LDFLAGS = -s
YACCF = -d
LFLAG1 = -i
LFLAG2 = -n
CFLAGS = -O -c $(DEFS) $(INCS)
CPASS1 =	cgram.c $(M)/comm1.c $(M)/optim.c $(M)/pftn.c $(M)/scan.c $(M)/trees.c \
		$(M)/xdefs.c xlocal.c lint.c $(M)/messages.c
OPASS1 =	cgram.o comm1.o optim.o pftn.o scan.o trees.o \
		xdefs.o xlocal.o lint.o messages.o

XPASS =	$(M)/cgram.y $(M)/comm1.c $(M)/common lint.c $(L)/lmanifest macdefs \
	$(M)/manifest $(M)/mfile1 $(M)/optim.c $(M)/pftn.c $(M)/scan.c $(M)/trees.c \
	$(M)/xdefs.c xlocal.c $(M)/messages.c $(M)/messages.h

all :	chk_pdp cxref xpass xcpp

chk_pdp :
	if pdp11 && [ "$(FLG)" != "x" ]; then \
		$(MAKE) -$(MAKEFLAGS) FLG=x DEFS="$(DEFS) -UFLEXNAME" \
			M="$(M)" L="$(L)" \
			CPP="$(CPP)" CC="$(CC)" \
			INCS="$(INCS)" -f cxref.mk all; \
	fi

# CXREF

cxref :	cxr.o
	$(CC) $(FFLAG) $(LDFLAGS) cxr.o -o cxref

cxr.o:	cxr.c owner.h
	$(CC) $(CFLAGS) cxr.c

# XPASS

xpass:	$(OPASS1)
	$(CC) $(FFLAG) $(LFLAG1) $(LDFLAGS) $(OPASS1) -o xpass

$(OPASS1): $(M)/manifest macdefs $(M)/mfile1

cgram.c: $(M)/cgram.y
	sed -e 's/\/\*CXREF\(.*\)\*\//\1/' $(M)/cgram.y > gram.y
	yacc gram.y
	mv y.tab.c cgram.c
	-rm -f gram.y

cgram.o:	cgram.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) cgram.c

comm1.o: $(M)/common
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) $(M)/comm1.c

lint.o:	$(L)/lmanifest lint.c
	$(CC) $(FFLAG) $(CFLAGS) lint.c
	
optim.o:	$(M)/optim.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) $(M)/optim.c
	
pftn.o:		$(M)/pftn.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) $(M)/pftn.c
	
scan.o: $(M)/scan.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) $(M)/scan.c

trees.o:	$(M)/trees.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) $(M)/trees.c

xdefs.o: $(M)/xdefs.c
	$(CC) -DBUG4 $(FFLAG) $(CFLAGS) $(M)/xdefs.c
	
xlocal.o:	xlocal.c $(L)/lmanifest
	$(CC) $(FFLAG) $(CFLAGS) xlocal.c

messages.o:	$(M)/messages.c $(M)/messages.h
	$(CC) $(FFLAG) $(CFLAGS) $(M)/messages.c

# XCPP

xcpp:	cpp.o cpy.o yylex.o
	$(CC) $(FFLAG) $(LFLAG2) $(LDFLAGS) -o xcpp cpp.o cpy.o yylex.o

cpp.o:	$(CPP)/cpp.c
	if u3b2; \
	then \
		$(CC) $(FFLAG) $(CFLAGS) -I$(CPP) -DPD_MACH=D_u3b2 -DPD_SYS=D_unix $(CPP)/cpp.c; \
	elif u3b5; \
	then \
		$(CC) $(FFLAG) $(CFLAGS) -I$(CPP) -DPD_MACH=D_u3b5 -DPD_SYS=D_unix $(CPP)/cpp.c; \
	elif u3b; \
	then \
		$(CC) $(FFLAG) $(CFLAGS) -I$(CPP) -DPD_MACH=D_u3b -DPD_SYS=D_unix $(CPP)/cpp.c; \
	elif u370; \
	then \
		$(CC) $(FFLAG) $(CFLAGS) -I$(CPP) -DPD_MACH=D_u370 -DPD_SYS=D_unix $(CPP)/cpp.c; \
	elif vax; \
	then \
		$(CC) $(FFLAG) $(CFLAGS) -I$(CPP) -DPD_MACH=D_vax -DPD_SYS=D_unix $(CPP)/cpp.c; \
	elif pdp11 ; \
	then \
		$(CC) $(FFLAG) $(CFLAGS) -I$(CPP) -DPD_MACH=D_pdp11 -DPD_SYS=D_unix $(CPP)/cpp.c; \
	fi

cpy.o:	cpy.c $(CPP)/yylex.c
	$(CC) $(FFLAG) $(CFLAGS) -I$(CPP) -Dunix=1 cpy.c

yylex.o:	$(CPP)/yylex.c y.tab.h
	$(CC) $(CFLAGS) -I$(CPP) $(CPP)/yylex.c

cpy.c:	$(CPP)/cpy.y
	yacc $(YACCF) $(CPP)/cpy.y
	mv y.tab.c cpy.c

# UTILITIES

install :	all
	cp cxref $(ROOT)/usr/bin
	cp xpass $(OWNER)
	cp xcpp $(OWNER)

clean:
	-rm -f *.o  

clobber:	clean
	-rm -f xpass cgram.c cxref xcpp cpy.c y.tab.h

lint:
	$(LINT) $(LINTF) cxr.c
	$(LINT) $(LINTF) -DBUG4 $(CPASS1)
	$(LINT) $(LINTF) -Dunix=1 $(CPP)/cpp.c cpy.c

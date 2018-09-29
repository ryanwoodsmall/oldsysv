#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libPW:libPW.mk	1.17"
ROOT=
CC=cc
LIB=$(ROOT)/lib
CFLAGS=-O 
STRIP=strip
LNAME = libPW.a

OBJECTS =	abspath.o anystr.o bal.o curdir.o fdfopen.o giveup.o \
	imatch.o index.o lockit.o logname.o move.o patoi.o \
	patol.o regcmp.o regex.o rename.o repeat.o repl.o satoi.o \
	setsig.o sname.o strend.o substr.o trnslat.o userdir.o \
	username.o verify.o any.o xalloc.o xcreat.o xlink.o \
	xopen.o xpipe.o xunlink.o xwrite.o xmsg.o alloca.o \
	cat.o dname.o fatal.o clean.o userexit.o zero.o zeropad.o

all:	init objects

init:
	@-if vax;\
	then\
		cp vax/move.s move.s;\
		cp vax/alloca.s alloca.s;\
	elif u370;\
	then\
		cp u370/move.c move.c;\
		cp u370/alloca.c alloca.c;\
	elif u3b;\
	then\
		cp u3b/move.c move.c;\
		cp u3b/alloca.s alloca.s;\
	elif u3b5 || u3b15 || u3b2;\
	then\
		cp m32/move.c move.c;\
		cp m32/alloca.s alloca.s;\
	else\
		cp pdp11/move.c move.c;\
		cp pdp11/alloca.s alloca.s;\
	fi

objects:	$(OBJECTS)
alloca.o:	alloca.s
	$(CC) -c alloca.s

$(LNAME):	all
	-rm -f $(LNAME)
	ar r $(LNAME) $(OBJECTS)
	-if pdp11 ; \
		then $(STRIP) $(LNAME) ; \
		else $(STRIP) -r $(LNAME) ; \
		     ar ts $(LNAME) ; \
	fi

install:	$(LNAME)
		cp $(LNAME) $(LIB)/$(LNAME)

clean:
	-rm -f $(OBJECTS)
	-rm -f move.c move.s alloca.c alloca.s $(LNAME)

clobber:	clean

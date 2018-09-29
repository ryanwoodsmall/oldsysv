#	@(#)libPW.mk	1.11
ROOT=
CC=cc
INS=install
INC=$(ROOT)/usr/include
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
	elif ns32000;\
	then\
		cp ns32000/move.s move.s;\
		cp ns32000/alloca.s alloca.s;\
	elif u370;\
	then\
		cp u370/move.c move.c;\
		cp u370/alloca.c alloca.c;\
	elif u3b;\
	then\
		cp u3b/move.c move.c;\
		cp u3b/alloca.s alloca.s;\
	elif u3b5;\
	then\
		cp u3b5/move.c move.c;\
		cp u3b5/alloca.s alloca.s;\
	else\
		cp pdp11/move.c move.c;\
		cp pdp11/alloca.s alloca.s;\
	fi

objects:	$(OBJECTS)
move.o:		move.s
	$(CC) -c move.s
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
	$(INS) -n $(ROOT)/usr/lib $(LNAME)

clean:
	-rm -f $(OBJECTS)
	-rm -f move.c move.s alloca.c alloca.s $(LNAME)

clobber:	clean

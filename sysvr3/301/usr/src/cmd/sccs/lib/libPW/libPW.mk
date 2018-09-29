#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:lib/libPW/libPW.mk	6.13"
#
#
ROOT =

AR = ar
LORDER = lorder

CFLAGS = -O

LIB = ../libPW.a

PRODUCTS = $(LIB)

FILES =	abspath.o	\
	alloca.o	\
	any.o	\
	anystr.o	\
	bal.o	\
	cat.o	\
	clean.o	\
	dname.o	\
	fatal.o	\
	fdfopen.o	\
	giveup.o	\
	imatch.o	\
	index.o	\
	lockit.o	\
	logname.o	\
	move.o	\
	patoi.o	\
	patol.o	\
	regcmp.o	\
	regex.o	\
	rename.o	\
	repeat.o	\
	repl.o	\
	satoi.o	\
	setsig.o	\
	sname.o	\
	strend.o	\
	substr.o	\
	trnslat.o	\
	userdir.o	\
	userexit.o	\
	username.o	\
	verify.o	\
	xalloc.o	\
	xcreat.o	\
	xlink.o	\
	xopen.o	\
	xpipe.o	\
	xunlink.o	\
	xwrite.o	\
	xmsg.o	\
	zero.o	\
	zeropad.o

all: $(LIB)
	@echo "Library $(PRODUCTS) is up to date"

$(LIB):	$(FILES)
	$(AR) cr $(LIB) `$(LORDER) *.o | tsort`
	$(CH) chmod 664 $(LIB)

move.o:	
	if vax; then	\
		$(CC) -c vax/move.s	; \
	elif u3b; then 	\
		$(CC) -c $(CFLAGS) u3b/move.c	; \
	elif u3b5; then	\
		$(CC) -c $(CFLAGS) u3b5/move.c	; \
	elif u3b2; then	\
		$(CC) -c $(CFLAGS) u3b2/move.c	; \
	elif pdp11; then	\
		$(CC) -c $(CFLAGS) pdp11/move.c	; \
	else	\
		$(CC) -c $(CFLAGS) u370/move.c	; \
	fi

alloca.o:
	if vax; then	\
		$(CC) -c vax/alloca.s	; \
	elif u3b; then 	\
		$(CC) -c u3b/alloca.s	; \
	elif u3b5; then	\
		$(CC)  -c u3b5/alloca.s	; \
	elif u3b2; then	\
		$(CC)  -c u3b2/alloca.s	; \
	elif pdp11; then	\
		$(CC) -c pdp11/alloca.s	; \
	else	\
		$(CC) -c $(CFLAGS) u370/alloca.c	; \
	fi

install:	$(LIB)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(PRODUCTS)

.PRECIOUS:	$(PRODUCTS)

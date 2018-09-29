#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:sdb.mk	1.20"

##	makefile for sdb

#	Hardware:
#		AT&T Technologies 3B20, 3B5, 3B2
#		DEC VAX 11/780
#	Operating Systems:
#		UNIX System V Release 2 (nonpaging and paging)

ROOT	=
BINDIR	=	$(ROOT)/bin
CC	=	cc
CFLAGS	=
FFLAG	=
TESTDIR	=	.
INSDIR	=
INS	=	/etc/install -n /usr/bin
FLEX	=	-DFLEXNAMES
#DEBUG=-DDEBUG for debugging info compilation
DEBUG	=

all:	
	if u3b2 ; then $(MAKE) -f sdb.mk			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" sdbs clean sdbp;	\
	else $(MAKE) -f sdb.mk sdb ;	\
	fi

sdb:
	if vax ; then cd vax ; 					\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b ; then cd u3b ; 				\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b5 ; then cd u3b5 ; 				\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b15 ; then cd u3b15 ; 				\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	fi

sdbp:
	if u3b2 ; then cd u3b2 ; 				\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
		mv sdb sdbp ;					\
	fi

sdbs:
	if u3b2 ; then cd u3b2 ; 				\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			INCSWP=../swinc				\
			CFLAGS="$(FFLAG) $(CFLAGS) -DSWAPPING"	\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
		mv sdb sdbs ;					\
	fi

install:
	if vax ; then cd vax ; 					\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b ; then cd u3b ; 				\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b5 ; then cd u3b5 ; 				\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b15 ; then cd u3b15 ; 				\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b2 ; then $(MAKE) -f sdb.mk ;			\
		cd u3b2 ; 					\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	fi

clean:
	if vax ; then cd vax ; 					\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b ; then cd u3b ; 				\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b5 ; then cd u3b5 ; 				\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b15 ; then cd u3b15 ; 				\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b2 ; then cd u3b2 ; 				\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	fi

clobber:	clean
	if vax ; then cd vax ; 					\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b ; then cd u3b ; 				\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b5 ; then cd u3b5 ; 				\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b15 ; then cd u3b15 ; 				\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b2 ; then cd u3b2 ; 				\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(FFLAG) $(CFLAGS)" 		\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	fi

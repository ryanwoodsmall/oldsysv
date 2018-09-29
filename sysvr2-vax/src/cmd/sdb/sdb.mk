#	@(#) sdb.mk: 1.13 6/20/84

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
TESTDIR	=	.
INSDIR	=
INS	=	/etc/install -n /usr/bin
FLEX	=	-DFLEXNAMES
#DEBUG=-DDEBUG for debugging info compilation
DEBUG	=

all:	sdb

sdb:
	if vax ; then cd vax ; 					\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(CFLAGS)" 			\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b ; then cd u3b ; 				\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(CFLAGS)" 			\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b5 ; then cd u3b5 ; 				\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="-f $(CFLAGS)" 			\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b2 ; then cd u3b2 ; 				\
		$(MAKE) -f makefile				\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="-f $(CFLAGS)" 			\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	fi

install:
	if vax ; then cd vax ; 					\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(CFLAGS)" 			\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b ; then cd u3b ; 				\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(CFLAGS)" 			\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b5 ; then cd u3b5 ; 				\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="-f $(CFLAGS)" 			\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b2 ; then cd u3b2 ; 				\
		$(MAKE) -f makefile install			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="-f $(CFLAGS)" 			\
			DEBUG="$(DEBUG)"			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	fi

clean:
	if vax ; then cd vax ; 					\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(CFLAGS)" 			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b ; then cd u3b ; 				\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(CFLAGS)" 			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b5 ; then cd u3b5 ; 				\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="-f $(CFLAGS)" 			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b2 ; then cd u3b2 ; 				\
		$(MAKE) -f makefile clean			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="-f $(CFLAGS)" 			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	fi

clobber:	clean
	if vax ; then cd vax ; 					\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(CFLAGS)" 			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b ; then cd u3b ; 				\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="$(CFLAGS)" 			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b5 ; then cd u3b5 ; 				\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="-f $(CFLAGS)" 			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	elif u3b2 ; then cd u3b2 ; 				\
		$(MAKE) -f makefile clobber			\
			FLEX="$(FLEX)"				\
			ROOT=$(ROOT) 				\
			CFLAGS="-f $(CFLAGS)" 			\
			TESTDIR="$(TESTDIR)" CC="$(CC)" ;	\
	fi

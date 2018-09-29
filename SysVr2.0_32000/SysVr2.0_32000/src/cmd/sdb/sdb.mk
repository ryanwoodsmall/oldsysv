#
#	@(#)sdb.mk	1.7

##	makefile for sdb
ROOT=
BINDIR=$(ROOT)/bin
CC = cc
CFLAGS =
TESTDIR = .
INSDIR =
INS = /etc/install -n /usr/bin
FLEX = -DFLEXNAMES
#		Hardware:	(not using)
#			vaxsdb - sdb for DEC VAX-11 
#			3b20sdb - sdb for BELL 3B20
#		Operating Systems:	(not using)
#			VAX135 - Department 1353; Bell Labs
#			UCBVAX - VM/UNIX 3.x and beyond Berkeley version
#			VFORK - Use vfork call of VM/UNIX Berkeley version
#		---->	STD - UNIX/32V, VM/UNIX 2.n, UNIX/TS, PWB UNIX, U3B
#	VM/UNIX 3.x uses new trap format which is shared with
#		VAX135 systems (head.h, setup.h).
##	-DDEBUG[=1]:	original sdb debugging.
##	-DDEBUG=2:	more robust debugging.
##	-DFLEXNAMES	is put on the cc line for 6.0 flexnames.
##	Now contains dependencies on /usr/include files.


#Depends on make_all, not sdb and libg.a, so that cd and make not done twice
all:	make_all

make_all:
	@if vax ; then cd vax ; \
		echo \\011 cd vax ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" CC="$(CC)" ;\
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" CC="$(CC)" ;\
		fi;
	@if ns32000 ; then cd ns32000 ; \
		echo \\011 cd ns32000 ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" CC="$(CC)" ;\
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" CC="$(CC)" ;\
		fi;
	@if u3b ; then cd u3b ; \
		echo \\011 cd u3b ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" \
		\ROOT=$(ROOT) \
		TESTDIR="$(TESTDIR)" CC="$(CC)" ;\
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" \
		ROOT=$(ROOT) \
		TESTDIR="$(TESTDIR)" CC="$(CC)" ;\
		fi;
	@if u3b5 ; then cd u3b5 ; \
		echo \\011 cd u3b5 ; \
		echo \\011 $(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="-f $(CFLAGS)" \
		ROOT=$(ROOT) \
		BINDIR=$(BINDIR) ; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="-f $(CFLAGS)" \
		ROOT=$(ROOT) \
		BINDIR=$(BINDIR) ; fi ;

sdb:
	@if vax ; then cd vax ; \
		echo \\011 cd vax ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../sdb ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../sdb ; fi;
	@if ns32000 ; then cd ns32000 ; \
		echo \\011 cd ns32000 ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../sdb ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../sdb ; fi;
	@if u3b ; then cd u3b ; \
		echo \\011 cd u3b ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../sdb ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../sdb ; fi;
	@if u3b5 ; then cd u3b5 ; \
		echo \\011 cd u3b5 ; \
		echo \\011 $(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)" \
		BINDIR=$(BINDIR) ../sdb ; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)" \
		BINDIR=$(BINDIR) ../sdb ; fi ;

libg.a:
	@if vax ; then cd vax ; \
		echo \\011 cd vax ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../libg.a ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../libg.a ; fi;
	@if ns32000 ; then cd ns32000; \
		echo \\011 cd ns32000 ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../libg.a ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../libg.a ; fi;
	@if u3b ; then cd u3b ; \
		echo \\011 cd u3b ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../libg.a ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	CC="$(CC)" ../libg.a ; fi;

install:
	@if vax ; then cd vax ; \
		echo \\011 cd vax ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	INSDIR="$(INSDIR)" CC="$(CC)" install ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	INSDIR="$(INSDIR)" CC="$(CC)" install ; fi ;
	@if ns32000 ; then cd ns32000 ; \
		echo \\011 cd ns32000 ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	INSDIR="$(INSDIR)" CC="$(CC)" install ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	INSDIR="$(INSDIR)" CC="$(CC)" install ; fi ;
	@if u3b ; then cd u3b ; \
		echo \\011 cd u3b ; \
		echo \\011 exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	INSDIR="$(INSDIR)" CC="$(CC)" install ; \
	exec make -f makefile CFLAGS="$(CFLAGS)" FLEX="$(FLEX)" TESTDIR="$(TESTDIR)" \
	INSDIR="$(INSDIR)" CC="$(CC)" install ; fi ;
	@if u3b5 ; then cd u3b5 ; \
		echo \\011 cd u3b5 ; \
		echo \\011 $(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)" \
		ROOT=$(ROOT) \
		BINDIR=$(BINDIR) install ; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" CFLAGS="$(CFLAGS)" \
		ROOT=$(ROOT) \
		BINDIR=$(BINDIR) install ; fi ;

clean :
	@if vax ; then cd vax ; \
		echo \\011 cd vax ; \
		echo \\011 exec make -f makefile TESTDIR="$(TESTDIR)" clean ; \
		exec make -f makefile TESTDIR="$(TESTDIR)" clean; \
	fi ;
	@if ns32000 ; then cd ns32000; \
		echo \\011 cd ns32000 ; \
		echo \\011 exec make -f makefile TESTDIR="$(TESTDIR)" clean ; \
		exec make -f makefile TESTDIR="$(TESTDIR)" clean; \
	fi ;
	@if u3b ; then cd u3b ; \
		echo \\011 cd u3b ; \
		echo \\011 exec make -f makefile TESTDIR="$(TESTDIR)" clean ; \
		exec make -f makefile TESTDIR="$(TESTDIR)" clean ; \
	fi ;
	@if u3b5 ; then cd u3b5 ; \
		echo \\011 cd u3b5 ; \
		echo \\011 $(MAKE) -$(MAKEFLAGS) clean ; \
		$(MAKE) -$(MAKEFLAGS) clean ; \
	fi ;

clobber:
	@if vax ; then cd vax ; \
		echo \\011 cd vax ; \
		echo \\011 exec make -f makefile TESTDIR="$(TESTDIR)" clobber ; \
		exec make -f makefile TESTDIR="$(TESTDIR)" clobber ; \
	fi ;
	@if ns32000 ; then cd ns32000 ; \
		echo \\011 cd ns32000 ; \
		echo \\011 exec make -f makefile TESTDIR="$(TESTDIR)" clobber ; \
		exec make -f makefile TESTDIR="$(TESTDIR)" clobber ; \
	fi ;
	@if u3b ; then cd u3b ; \
		echo \\011 cd u3b ; \
		echo \\011 exec make -f makefile TESTDIR="$(TESTDIR)" clobber ; \
		exec make -f makefile TESTDIR="$(TESTDIR)" clobber ; \
	fi ;
	@if u3b5 ; then cd u3b5 ; \
		echo \\011 cd u3b5 ; \
		echo \\011 $(MAKE) -$(MAKEFLAGS) clobber ; \
		$(MAKE) -$(MAKEFLAGS) clobber ; \
	fi ;

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)f77:f77.mk	1.13.2.1"
#	@(#)f77:f77.mk	1.13.2.1
#	F77 GLOBAL MAKEFILE

all:
	-if u3b; then \
		cd u3b/pass1; make -f pass1.mk; \
		cd  ../pass2; make -f pass2.mk; \
		cd   ../../optim/u3b; make -f f77optim.mk; exit; \
	elif vax; then \
		cd vax/pass1; make -f pass1.mk; \
		cd  ../pass2; make -f pass2.mk; exit; \
	elif pdp11; then \
		cd pdp11; make -f f77.mk; exit; \
	elif u3b5 || u3b2 || u3b15; then \
		cd u3b5/pass1; make -f pass1.mk; \
		cd   ../pass2; make -f pass2.mk; \
		cd   ../../optim/om32; make -f f77optim.mk; exit; fi;

install:
	-if u3b; then \
		cd u3b/pass1; make -f pass1.mk install; \
		cd  ../pass2; make -f pass2.mk install; \
		cd   ../../optim/u3b; make -f f77optim.mk install; exit; \
	elif vax; then \
		cd vax/pass1; make -f pass1.mk install; \
		cd  ../pass2; make -f pass2.mk install; exit; \
	elif pdp11; then \
		cd pdp11; make -f f77.mk; \
			  make -f f77.mk install; exit; \
	elif u3b5 || u3b2 || u3b15; then \
		cd u3b5/pass1; make -f pass1.mk install; \
		cd   ../pass2; make -f pass2.mk install; \
		cd   ../../optim/om32; make -f f77optim.mk install; exit; fi;

clean:
	-if u3b; then \
		cd u3b/pass1; make -f pass1.mk clean; \
		cd  ../pass2; make -f pass2.mk clean; \
		cd   ../../optim/u3b; make -f f77optim.mk clean; exit; \
	elif vax; then \
		cd vax/pass1; make -f pass1.mk clean; \
		cd  ../pass2; make -f pass2.mk clean; exit; \
	elif pdp11; then \
		cd pdp11; make -f f77.mk clean; exit; \
	elif u3b5 || u3b2 || u3b15; then \
		cd u3b5/pass1; make -f pass1.mk clean; \
		cd   ../pass2; make -f pass2.mk clean; \
		cd   ../../optim/om32; make -f f77optim.mk clean; exit; fi;

clobber:
	-if u3b; then \
		cd u3b/pass1; make -f pass1.mk clobber; \
		cd  ../pass2; make -f pass2.mk clobber; \
		cd   ../../optim/u3b; make -f f77optim.mk clobber; exit; \
	elif vax; then \
		cd vax/pass1; make -f pass1.mk clobber; \
		cd  ../pass2; make -f pass2.mk clobber; exit; \
	elif pdp11; then \
		cd pdp11; make -f f77.mk clobber; exit; \
	elif u3b5 || u3b2 || u3b15; then \
		cd u3b5/pass1; make -f pass1.mk clobber; \
		cd   ../pass2; make -f pass2.mk clobber; \
		cd   ../../optim/om32; make -f f77optim.mk clobber; exit; fi;

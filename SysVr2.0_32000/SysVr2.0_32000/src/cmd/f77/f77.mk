#	@(#)f77.mk	1.8
#	F77 GLOBAL MAKEFILE

all:
	-if u3b; then \
		cd u3b/pass1; make -f pass1.mk; \
		cd  ../pass2; make -f pass2.mk; exit; \
	elif u3b5; then \
		cd u3b5/pass1; make -f pass1.mk; \
		cd   ../pass2; make -f pass2.mk; exit; \
	elif vax; then \
		cd vax/pass1; make -f pass1.mk; \
		cd  ../pass2; make -f pass2.mk; exit; \
	elif ns32000; then \
		cd ns32000/pass1; make -f pass1.mk; \
		cd ../pass2; make -f pass2.mk; exit; \
	elif pdp11; then \
		cd pdp11; make -f f77.mk; exit; fi;

install:
	-if u3b; then \
		cd u3b/pass1; make -f pass1.mk; \
			      make -f pass1.mk install; \
		cd  ../pass2; make -f pass2.mk; \
			      make -f pass2.mk install; exit; \
	elif u3b5; then \
		cd u3b5/pass1; make -f pass1.mk; \
			       make -f pass1.mk install; \
		cd   ../pass2; make -f pass2.mk; \
			       make -f pass2.mk install; exit; \
	elif vax; then \
		cd vax/pass1; make -f pass1.mk; \
			      make -f pass1.mk install; \
		cd  ../pass2; make -f pass2.mk; \
			      make -f pass2.mk install; exit; \
	elif ns32000; then \
		cd ns32000/pass1; make -f pass1.mk; \
			      make -f pass1.mk install; \
		cd  ../pass2; make -f pass2.mk; \
			      make -f pass2.mk install; exit; \
	elif pdp11; then \
		cd pdp11; make -f f77.mk; \
			  make -f f77.mk install; exit; fi;

clean:
	-if u3b; then \
		cd u3b/pass1; make -f pass1.mk clean; \
		cd  ../pass2; make -f pass2.mk clean; exit; \
	elif u3b5; then \
		cd u3b5/pass1; make -f pass1.mk clean; \
		cd   ../pass2; make -f pass2.mk clean; exit; \
	elif vax; then \
		cd vax/pass1; make -f pass1.mk clean; \
		cd  ../pass2; make -f pass2.mk clean; exit; \
	elif ns32000; then \
		cd ns32000/pass1; make -f pass1.mk clean; \
		cd  ../pass2; make -f pass2.mk clean; exit; \
	elif pdp11; then \
		cd pdp11; make -f f77.mk clean; exit; fi;

clobber:
	-if u3b; then \
		cd u3b/pass1; make -f pass1.mk clobber; \
		cd  ../pass2; make -f pass2.mk clobber; exit; \
	elif u3b5; then \
		cd u3b5/pass1; make -f pass1.mk clobber; \
		cd   ../pass2; make -f pass2.mk clobber; exit; \
	elif vax; then \
		cd vax/pass1; make -f pass1.mk clobber; \
		cd  ../pass2; make -f pass2.mk clobber; exit; \
	elif ns32000; then \
		cd ns32000/pass1; make -f pass1.mk clobber; \
		cd  ../pass2; make -f pass2.mk clobber; exit; \
	elif pdp11; then \
		cd pdp11; make -f f77.mk clobber; exit; fi;

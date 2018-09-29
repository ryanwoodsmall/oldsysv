# @(#)Makefile	1.1
I =	../../include
LIB =	../../lib

glib::	$(I) gsl.d; cd gsl.d; make LIB=../$(LIB); touch ../glib
glib::	$(I) gpl.d; cd gpl.d; make LIB=../$(LIB); touch ../glib

listings:	gsl.d gpl.d
		cd gsl.d; make -s listings
		cd gpl.d; make -s listings
		touch listings

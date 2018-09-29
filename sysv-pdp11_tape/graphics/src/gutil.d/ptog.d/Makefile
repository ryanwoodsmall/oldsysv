# @(#)Makefile	1.2
I =	../../../include
BIN =	../../../bin
SOURCE = ptog.c linemod.c ptogarcpt.c
LIB =	../../../lib/glib.a
HDR =	$(I)/util.h $(I)/gpl.h $(I)/gsl.h $(I)/errpr.h

ptog:	ptog.a $(LIB)
	cc $(FFLAG) ptog.a $(LIB) -lm -s -o $(BIN)/ptog
	chmod 755 $(BIN)/ptog
	touch ptog

ptog.a:: $(HDR) ptog.c; sh $(SHFLAG) ./cca ptog.c
ptog.a:: linemod.c; sh $(SHFLAG) ./cca linemod.c
ptog.a:: ptogarcpt.c; sh $(SHFLAG) ./cca ptogarcpt.c

listings: $(HDR) $(SOURCE)  
	ls -l *.c *.h | pr -h "  gutil.d/ptog.d source files  "
	print $? 
	touch listings

# @(#)Makefile	1.2
I =	../../../include
LIB =	../../../lib
SOURCE = box.c gsl.c gslgsl.c gslswing.c \
	putx0y0.c region.c textbox.c textcent.c \
	textright.c xymaxmin.c
HDR0 =	$(I)/gsl.h
HDR1 =	$(HDR0) $(I)/errpr.h $(I)/util.h
HDR2 =	$(HDR1) $(I)/gpl.h

gsl::	$(HDR0) textbox.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca textbox.c; touch gsl
gsl::	$(HDR0) box.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca box.c; touch gsl
gsl::	$(HDR0) textright.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca textright.c; touch gsl
gsl::	$(HDR0) textcent.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca textcent.c; touch gsl
gsl::	$(HDR2) xymaxmin.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca xymaxmin.c; touch gsl
gsl::	$(HDR1) gsl.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gsl.c; touch gsl
gsl::	$(HDR2) gslgsl.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gslgsl.c; touch gsl
gsl::	$(HDR2) putx0y0.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca putx0y0.c; touch gsl
gsl::	$(HDR0) gslswing.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gslswing.c; touch gsl
gsl::	$(HDR1) region.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca region.c; touch gsl

listings: $(HDR2) $(SOURCE)
	ls -l *.c *.h | pr -h "     gsl.d     "
	print $?
	touch listings

# @(#)Makefile	1.2
I =	../../../include
SOURCE = getgedf.c getgeds.c gplalpha.c \
	gplarcs.c gplcomment.c gplinit.c gpllines.c  \
	gpltext.c putgedf.c putgeds.c \
	getgeds.c putgeds.c
HDR0 =	$(I)/gpl.h
HDR1 =	$(HDR0) $(I)/util.h
LIB =	../../../lib

gpl::	$(HDR1) gplinit.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gplinit.c; touch gpl
gpl::	$(HDR0) gplarcs.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gplarcs.c; touch gpl
gpl::	$(HDR0) gplcomment.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gplcomment.c; touch gpl
gpl::	$(HDR0) gpllines.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gpllines.c; touch gpl
gpl::	$(HDR0) gpltext.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gpltext.c; touch gpl
gpl::	$(HDR0) gplalpha.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca gplalpha.c; touch gpl
gpl::	$(HDR1) getgedf.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca getgedf.c; touch gpl
gpl::	$(HDR1) putgedf.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca putgedf.c; touch gpl
gpl::	$(HDR1) getgeds.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca getgeds.c; touch gpl
gpl::	$(HDR1) putgeds.c; LIB=$(LIB)/glib.a sh $(SHFLAG) ./cca putgeds.c; touch gpl

listings: $(HDR1) $(SOURCE)
	ls -l *.c *.h | pr -h "     gpl.d     "
	print $?
	touch listings

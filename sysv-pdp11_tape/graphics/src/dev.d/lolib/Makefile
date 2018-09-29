# @(#)Makefile	1.2
I =	../../../include
L =	../../../lib
SOURCE = adjarc.c adjline.c  adjtext.c
HDR1 =	$(I) ../include/dev.h

dev:  lolib.a  $(L)/glib.a
	touch dev

lolib.a::	$(HDR1) adjarc.c; sh $(SHFLAG) ./cca adjarc.c
lolib.a::	$(HDR1) adjline.c; sh $(SHFLAG) ./cca adjline.c
lolib.a::	$(HDR1) adjtext.c; sh $(SHFLAG) ./cca adjtext.c

listings: $(HDR1) $(SOURCE)  
	ls -l  *.c *.h | pr -h "  dev.d/lib2 source files  "
	pr $? 
	touch listings

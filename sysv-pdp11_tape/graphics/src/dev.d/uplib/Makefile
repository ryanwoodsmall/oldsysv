# @(#)Makefile	1.2
I =	../../../include
LIB =	../../../lib/glib.a
SOURCE =  arcfill.c clipseg.c compwind.c dispfile.c dispprim.c  inarea.c \
	linefill.c softarc.c softtext.c utodcoord.c
HDR1 =	$(I) ../include/dev.h
HDR2 = $(HDR1) ../include/fill.h

uplib:  uplib.a  $(LIB)
	touch uplib

uplib.a::	$(HDR1) compwind.c; sh $(SHFLAG) ./cca compwind.c
uplib.a::	$(HDR1) dispfile.c; sh $(SHFLAG) ./cca dispfile.c
uplib.a::	$(HDR1) utodcoord.c; sh $(SHFLAG) ./cca utodcoord.c
uplib.a::	$(HDR1) dispprim.c; sh $(SHFLAG) ./cca dispprim.c
uplib.a::	$(HDR2) linefill.c; sh $(SHFLAG) ./cca linefill.c
uplib.a::	$(HDR2) arcfill.c; sh $(SHFLAG) ./cca arcfill.c
uplib.a::	$(HDR1) softarc.c; sh $(SHFLAG) ./cca softarc.c
uplib.a::	$(HDR1) softtext.c; sh $(SHFLAG) ./cca softtext.c
uplib.a::	$(HDR1) clipseg.c; sh $(SHFLAG) ./cca clipseg.c
uplib.a::	$(HDR1) inarea.c; sh $(SHFLAG) ./cca inarea.c

listings: $(HDR1) $(SOURCE)  
	ls -l  *.c *.h | pr -h "  dev.d/lib source files  "
	pr $? 
	touch listings

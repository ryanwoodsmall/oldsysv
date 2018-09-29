# @(#)Makefile	1.3
I =	../../../../include
L =	../../../../lib
BIN =	../../../../bin
LIB =	../lib
HDRS =	$(I) ged.h ../include/gedstructs.h
SOURCE = bufged.c clipu.c cmdfuncs.c cmdproc.c copymove.c deleteobj.c dispbuf.c drbox.c \
	ged.c getfile.c \
	objects.c outwindow.c points.c \
	rotarea.c scalearea.c setwindow.c text.c \
	unixesc.c window.c xymxmn.c

ged::	ged.a $(LIB)/tek.a $(L)/glib.a
	cc $(FFLAG) ged.a $(LIB)/tek.a $(L)/glib.a -lm -i -s -o $(BIN)/ged
	chmod 755 $(BIN)/ged
	touch ged

ged.a::	$(HDRS) ged.c; sh $(SHFLAG) ./cca ged.c
ged.a::	$(HDRS) cmdproc.c
	if u370 ; then sh $(SHFLAG) ./cca "-b1,0 cmdproc.c" ; \
	else sh $(SHFLAG) ./cca cmdproc.c ; fi
ged.a:: $(HDRS) cmdfuncs.c; sh $(SHFLAG) ./cca cmdfuncs.c
ged.a::	$(HDRS) dispbuf.c; sh $(SHFLAG) ./cca dispbuf.c
ged.a::	$(HDRS) setwindow.c; sh $(SHFLAG) ./cca setwindow.c
ged.a::	$(HDRS) outwindow.c; sh $(SHFLAG) ./cca outwindow.c
ged.a::	$(HDRS) getfile.c; sh $(SHFLAG) ./cca getfile.c
ged.a::	$(HDRS) text.c; sh $(SHFLAG) ./cca text.c
ged.a::	$(HDRS) drbox.c; sh $(SHFLAG) ./cca drbox.c
ged.a::	$(HDRS) unixesc.c; sh $(SHFLAG) ./cca unixesc.c
ged.a::	$(HDRS) objects.c; sh $(SHFLAG) ./cca objects.c
ged.a::	$(HDRS) points.c; sh $(SHFLAG) ./cca points.c
ged.a::	$(HDRS) deleteobj.c; sh $(SHFLAG) ./cca deleteobj.c
ged.a:: $(HDRS) copymove.c; sh $(SHFLAG) ./cca copymove.c
ged.a::	$(HDRS) rotarea.c; sh $(SHFLAG) ./cca rotarea.c
ged.a::	$(HDRS) scalearea.c; sh $(SHFLAG) ./cca scalearea.c
ged.a::	$(HDRS) bufged.c; sh $(SHFLAG) ./cca bufged.c
ged.a::	$(HDRS) window.c; sh $(SHFLAG) ./cca window.c
ged.a::	$(HDRS) xymxmn.c; sh $(SHFLAG) ./cca xymxmn.c
ged.a:: $(HDRS) clipu.c; sh $(SHFLAG) ./cca clipu.c

listings: $(HDRS) $(SOURCE)  
	ls -l *.c *.h | pr -h "  tek4000/ged.d source files  "
	print $? 
	touch listings

# @(#)Makefile	1.2
I =	../../../../include
HDR1 =	$(I) tek.h  ../include/gedstructs.h
HDR2 = $(HDR1)  ../include/termodes.h

SOURCE = addrbuf.c chplot.c circarc.c clip.c \
	getacur.c getcraw.c getline.c getpoint.c getpos.c inarea.c locacur.c\
	scaleoff.c setmode.c\
	tekalpha.c tekarc.c tekflush.c tekinit.c teklines.c tekmove.c\
	tektext.c tekxy.c unscaleoff.c xytek.c xytekl.c

tek:	tek.a
	touch tek

tek.a::	$(HDR1) teklines.c; sh $(SHFLAG) ./cca teklines.c
tek.a::	$(HDR2) tekmove.c; sh $(SHFLAG) ./cca tekmove.c
tek.a::	$(HDR2) tekinit.c; sh $(SHFLAG) ./cca tekinit.c
tek.a::	$(HDR1) tektext.c; sh $(SHFLAG) ./cca tektext.c
tek.a::	$(HDR2) tekalpha.c; sh $(SHFLAG) ./cca tekalpha.c
tek.a::	$(HDR1) tekarc.c; sh $(SHFLAG) ./cca tekarc.c
tek.a::	$(HDR1) chplot.c; sh $(SHFLAG) ./cca chplot.c
tek.a::	$(HDR1) circarc.c; sh $(SHFLAG) ./cca circarc.c
tek.a::	$(HDR2) addrbuf.c; sh $(SHFLAG) ./cca addrbuf.c
tek.a::	$(HDR1) clip.c; sh $(SHFLAG) ./cca clip.c
tek.a::	$(HDR2) tekflush.c; sh $(SHFLAG) ./cca tekflush.c
tek.a::	$(HDR2) setmode.c; sh $(SHFLAG) ./cca setmode.c
tek.a::	$(HDR2) getacur.c; sh $(SHFLAG) ./cca getacur.c
tek.a::	$(HDR2) locacur.c; sh $(SHFLAG) ./cca locacur.c
tek.a::	$(HDR2) getpos.c; sh $(SHFLAG) ./cca getpos.c
tek.a::	$(HDR1) tekxy.c; sh $(SHFLAG) ./cca tekxy.c
tek.a::	$(HDR1) xytek.c; sh $(SHFLAG) ./cca xytek.c
tek.a::	$(HDR1) xytekl.c; sh $(SHFLAG) ./cca xytekl.c
tek.a::	$(HDR1) inarea.c; sh $(SHFLAG) ./cca inarea.c
tek.a:: $(HDR1) getcraw.c; sh $(SHFLAG) ./cca getcraw
tek.a:: $(HDR1) getline.c; sh $(SHFLAG) ./cca getline
tek.a:: $(HDR2) getpoint.c; sh $(SHFLAG) ./cca getpoint
tek.a::	$(HDR1) scaleoff.c; sh $(SHFLAG) ./cca scaleoff.c
tek.a::	$(HDR1) unscaleoff.c; sh $(SHFLAG) ./cca unscaleoff.c


listings:	$(HDR2) $(SOURCE)  
	ls -l  *.c *.h | pr -h "  tek4000/lib source files  "
	print $? 
	touch listings

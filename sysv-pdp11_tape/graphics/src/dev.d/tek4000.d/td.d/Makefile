# @(#)Makefile	1.2
I =	../../../../include
L =	../../../../lib
BIN = ../../../../bin
UPLIB = ../../uplib
LOLIB = ../../lolib
SOURCE =	devarc.c devcursor.c devfinal.c devinit.c devline.c \
		devtext.c xytek.c xytekl.c devfill.c devbuff.c

HDR1 =	$(I) ../../include/dev.h dev.h

dev:	dev.c dev.a $(UPLIB)/uplib.a $(LOLIB)/lolib.a  $(L)/glib.a
	cc $(FFLAG) dev.c $(UPLIB)/uplib.a dev.a $(LOLIB)/lolib.a  $(L)/glib.a -lm -i -s -o $(BIN)/td
	chmod 755 $(BIN)/td
	touch td

dev.a::	$(HDR1) devinit.c; sh $(SHFLAG) ./cca devinit.c
dev.a::	$(HDR1) devfinal.c; sh $(SHFLAG) ./cca devfinal.c
dev.a::	$(HDR1) devcursor.c; sh $(SHFLAG) ./cca devcursor.c
dev.a::	$(HDR1) devarc.c; sh $(SHFLAG) ./cca devarc.c
dev.a::	$(HDR1) devline.c; sh $(SHFLAG) ./cca devline.c
dev.a::	$(HDR1) devtext.c; sh $(SHFLAG) ./cca devtext.c
dev.a::	$(HDR1) xytek.c; sh $(SHFLAG) ./cca xytek.c
dev.a::	$(HDR1) xytekl.c; sh $(SHFLAG) ./cca xytekl.c
dev.a:: $(HDR1) devfill.c; sh $(SHFLAG) ./cca devfill.c
dev.a:: $(HDR1) devbuff.c; sh $(SHFLAG) ./cca devbuff.c

listings: $(HDR1) $(SOURCE)  
	ls -l  *.c *.h | pr -h "  dev.d/tek4000.d/td.d source files  "
	pr $? 
	touch listings

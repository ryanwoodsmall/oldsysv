# @(#)Makefile	1.2
I =	../../../../include
L =	../../../../lib
BIN = ../../../../bin
UPLIB = ../../uplib
LOLIB = ../../lolib
SOURCE =	devarc.c devbuff.c devcursor.c devfinal.c devinit.c devfill.c \
		devline.c devtext.c mba.c mbn.c mbp.c newcolor.c sbn.c

HDR1 =	$(I) ../../include/dev.h dev.h

hp:	dev.c dev.a $(UPLIB)/uplib.a $(LOLIB)/lolib.a  $(L)/glib.a
	cc $(FFLAG) dev.c $(UPLIB)/uplib.a dev.a $(LOLIB)/lolib.a  $(L)/glib.a -lm -i -s -o $(BIN)/hpd
	chmod 755 $(BIN)/hpd
	touch hp

dev.a::	$(HDR1) devinit.c; sh $(SHFLAG) ./cca devinit.c
dev.a:: $(HDR1) devbuff.c; sh $(SHFLAG) ./cca devbuff.c
dev.a::	$(HDR1) devfinal.c; sh $(SHFLAG) ./cca devfinal.c
dev.a::	$(HDR1) devcursor.c; sh $(SHFLAG) ./cca devcursor.c
dev.a::	$(HDR1) devarc.c; sh $(SHFLAG) ./cca devarc.c
dev.a::	$(HDR1) devline.c; sh $(SHFLAG) ./cca devline.c
dev.a::	$(HDR1) devtext.c; sh $(SHFLAG) ./cca devtext.c
dev.a::	$(HDR1) newcolor.c; sh $(SHFLAG) ./cca newcolor.c
dev.a::	$(HDR1) mbp.c; sh $(SHFLAG) ./cca mbp.c
dev.a::	$(HDR1) mba.c; sh $(SHFLAG) ./cca mba.c
dev.a::	$(HDR1) mbn.c; sh $(SHFLAG) ./cca mbn.c
dev.a::	$(HDR1) sbn.c; sh $(SHFLAG) ./cca sbn.c
dev.a:: $(HDR1) devfill.c; sh $(SHFLAG) ./cca devfill.c

listings: $(HDR1) $(SOURCE)  
	ls -l  *.c *.h | pr -h "  dev.d/hp.d source files  "
	pr $? 
	touch listings

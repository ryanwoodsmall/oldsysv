#	makefile for aps-5 driver, fonts, etc.
#
# DSL 2
#  SCCS @(#)devaps.mk	1.12

OL = $(ROOT)/
CFLAGS = -O
LDFLAGS = -s
INS = :
INSDIR = $(OL)usr/bin
FONTDIR = $(OL)usr/lib/font/devaps
MAKEDEV = ./makedev
FFILES = [A-Z] [A-Z][0-9A-Z] DESC
OFILES = [A-Z].[oa][ud][td] [A-Z][0-9A-Z].[oa][ud][td] DESC.out

all:	daps aps_fonts

daps:	daps.o ../draw.o
	$(CC) $(LDFLAGS) -o daps daps.o ../draw.o -lm
	$(INS)  daps $(INSDIR)
	cd $(INSDIR); chmod 755 daps; $(CH) chgrp bin daps; chown bin daps

daps.o:	aps.h ../dev.h daps.h daps.g

./makedev:	makedev.c ../dev.h
	cc $(LDFLAGS) -o makedev makedev.c

aps_fonts:	makedir $(MAKEDEV)
	$(MAKEDEV) DESC
	for i in $(FFILES); \
	do	if [ ! -r $$i.out ] || [ -n "`find $$i -newer $$i.out -print`" ]; \
		   then	$(MAKEDEV) $$i; \
		fi; \
	done
	$(INS) $(OFILES) version $(FONTDIR);
	cd $(FONTDIR); chmod 644 $(OFILES) version;  \
		$(CH) chgrp bin $(OFILES) version; chown bin $(OFILES) version

makedir:
	if [ ! -d $(FONTDIR) ] ; then rm -f $(FONTDIR);  mkdir $(FONTDIR) ;  fi

install:
	$(MAKE) -f devaps.mk INS=cp all ROOT=$(ROOT) CH=$(CH)

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILES) daps makedev

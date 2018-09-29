#	makefile for canon/imagen Imprint-10 driver, fonts, etc.
#
# DSL 2
#  SCCS @(#)devi10.mk	1.10

OL = $(ROOT)/
CFLAGS = -O
LDFLAGS = -s
INS = :
INSDIR = $(OL)usr/bin
FONTDIR = $(OL)usr/lib/font/devi10
MAKEDEV = ../makedev
FFILES = [A-Z] [A-Z][0-9A-Z] DESC
OFILES = [A-Z].out [A-Z][0-9A-Z].out DESC.out

all:	di10 i10_fonts

di10:	di10.o ../draw.o
	$(CC) $(LDFLAGS) -o di10 di10.o ../draw.o -lm
	$(INS) di10 $(INSDIR)
	cd $(INSDIR); chmod 775 di10; $(CH) chgrp bin di10; chown bin di10

di10.o:	../dev.h canon.h glyph.h spectab.h

i10_fonts:	makedir $(MAKEDEV)
	$(MAKEDEV) DESC
	for i in $(FFILES); \
	do	if [ ! -r $$i.out ] || [ -n "`find $$i -newer $$i.out -print`" ]; \
		   then	$(MAKEDEV) $$i; \
		fi; \
	done
	$(INS) $(OFILES) $(FONTDIR);
	cd $(FONTDIR); chmod 644 $(OFILES);  \
		$(CH) chgrp bin $(OFILES); chown bin $(OFILES)

makedir:
	if [ ! -d $(FONTDIR) ] ; then rm -f $(FONTDIR);  mkdir $(FONTDIR) ;  fi

$(MAKEDEV):
	cc -s -o $(MAKEDEV) $(MAKEDEV).c

makei10:	makei10.c ../dev.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o makei10 makei10.c

makefonts:	makefonts.c ../dev.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o makefonts makefonts.c

install:
	$(MAKE) -f devi10.mk INS=cp all ROOT=$(ROOT) CH=$(CH)
	cd rasti10; $(MAKE) -f rasti10.mk install ROOT=$(ROOT) CH=$(CH)

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILES) di10 makei10 makefonts
	cd rasti10;  $(MAKE) -f rasti10.mk clobber

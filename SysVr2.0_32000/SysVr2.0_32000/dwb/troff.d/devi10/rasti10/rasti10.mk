#	makefile for raster tables for Imagen Imprint-10 typesetter
#
# DSL 2
#  SCCS @(#)rasti10.mk	1.13

OL = $(ROOT)/
CFLAGS = -O
LDFLAGS = -s
INS = :
RASTDIR = $(OL)usr/lib/font/devi10/rasti10
FILES = [A-Z].[0-9]* [A-Z][A-Z0-9].[0-9]*
3BRASTDIR = 3b-rast

all i10_rasts:	make3brast
	if u3b ; \
	  then	if [ ! -d $(3BRASTDIR) ]; \
			then rm -f $(3BRASTDIR); mkdir $(3BRASTDIR); \
		fi ; \
		for i in $(FILES); \
		do	if [ ! -r $(3BRASTDIR)/$$i ] || [ -n "`find $$i -newer $(3BRASTDIR)/$$i -print`" ]; \
			   then	echo $$i;  ./make3brast $$i > $(3BRASTDIR)/$$i; \
			fi; \
		done; \
	fi
	if [ ! -d $(RASTDIR) ] ; then rm -f $(RASTDIR);  mkdir $(RASTDIR);  fi
	if u3b;  then cd $(3BRASTDIR);  $(INS) $(FILES) $(RASTDIR); \
		 else  $(INS) $(FILES) $(RASTDIR); \
	fi
	$(INS) RASTERLIST $(RASTDIR)
	cd $(RASTDIR); chmod 644 $(FILES) RASTERLIST; \
		$(CH) chgrp bin $(FILES) RASTERLIST; chown bin $(FILES) RASTERLIST

make3brast:	make3brast.c
	cc $(CFLAGS) $(LDFLAGS) -o make3brast make3brast.c

fdump:	fdump.c ../glyph.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o fdump fdump.c

fbuild:	fbuild.c ../glyph.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o fbuild fbuild.c

install:
	$(MAKE) -f rasti10.mk INS=cp all ROOT=$(ROOT) CH=$(CH)
	cd aps-i10; $(MAKE) -f aps-i10.mk install ROOT=$(ROOT) CH=$(CH)

clean:
	rm -f *.o
	cd aps-i10;  $(MAKE) -f aps-i10.mk clean

clobber:	clean
	rm -f make3brast fdump fbuild
	if u3b;  then rm -rf $(3BRASTDIR);  fi
	cd aps-i10;  $(MAKE) -f aps-i10.mk clobber

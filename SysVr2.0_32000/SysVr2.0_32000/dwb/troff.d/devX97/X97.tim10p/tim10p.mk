#	makefile for times roman 10 pitch fonts for the xerox 9700.
#
# DSL 2
#  SCCS @(#)tim10p.mk	1.10

OL = $(ROOT)/
INS = :
FONTDIR = $(OL)usr/lib/font/devX97.tim10p
MAKEDEV = ../../makedev
FFILES = [A-Z] [A-Z][0-9A-Z] DESC
OFILES = [A-Z].out [A-Z][0-9A-Z].out DESC.out
MAKE = make

all tim10p_fonts:	makedir $(MAKEDEV)
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

install:
	${MAKE} -f tim10p.mk INS=cp ROOT=$(ROOT) CH=$(CH)

clean:
	:

clobber:	clean
	rm -f $(OFILES)

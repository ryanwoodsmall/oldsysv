#	@(#)makefile	2.3

#***************************************************************************
#                                                                          *
#		UNIX  pdp-11 Strip (strip) makefile			   *
#									   *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#		strip.c		main program for pdp11 version of strip	   *
#									   *
#***************************************************************************

LPT = dpr -mx

INSDIR = .

CFILES = strip.c

CFLAGS = -O -s -i


all:	strip


strip:	$(CFILES)
	$(CC) $(CFLAGS) strip.c -o strip
	cp strip $(INSDIR)/strip


listing:
	pr -n makefile $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f strip.o strip

clobber: clean
	-rm -f strip

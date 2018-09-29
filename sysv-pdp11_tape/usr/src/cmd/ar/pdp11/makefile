#	@(#)makefile	2.2

#***************************************************************************
#                                                                          *
#		UNIX  pdp-11 Archive (ar) makefile			   *
#									   *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#		ar.c		main program for pdp11 version of ar	   *
#									   *
#***************************************************************************

LPT = dpr -mx

INSDIR = .

CFILES = ar.c

CFLAGS = -O -s -i


all:	ar


ar:	$(CFILES)
	$(CC) $(CFLAGS) ar.c -o ar
	cp ar $(INSDIR)/ar


listing:
	pr -n makefile $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f ar ar.o

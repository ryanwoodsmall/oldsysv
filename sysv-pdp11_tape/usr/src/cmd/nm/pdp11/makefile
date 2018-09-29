#	@(#)makefile	2.3

#***************************************************************************
#                                                                          *
#		UNIX  pdp-11 Nm (nm) makefile				   *
#									   *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#		nm.c		main program for pdp11 version of nm	   *
#									   *
#***************************************************************************

LPT = dpr -mx

INSDIR = .

CFILES = nm.c

CFLAGS = -O -s -i


all:	nm


nm:	$(CFILES)
	$(CC) $(CFLAGS) nm.c -o nm
	cp nm $(INSDIR)/nm


listing:
	pr -n makefile $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f nm.o nm

clobber: clean
	-rm -f nm

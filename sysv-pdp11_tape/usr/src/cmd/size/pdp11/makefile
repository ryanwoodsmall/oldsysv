#	@(#)makefile	2.3

#***************************************************************************
#                                                                          *
#		UNIX  pdp-11 Size (size) makefile			   *
#									   *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#		size.c		main program for pdp11 version of size	   *
#									   *
#***************************************************************************

LPT = dpr -mx

INSDIR = .

CFILES = size.c

CFLAGS = -O -s -i


all:	size


size:	$(CFILES)
	$(CC) $(CFLAGS) size.c -o size
	cp size $(INSDIR)/size


listing:
	pr -n makefile $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f size.o size

clobber: clean
	-rm -f size

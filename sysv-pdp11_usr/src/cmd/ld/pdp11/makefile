#	@(#)makefile	2.3

#***************************************************************************
#                                                                          *
#		UNIX  pdp-11 Link-editor (ld) makefile			   *
#									   *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#		ld.c		main program for pdp11 version of ld	   *
#									   *
#***************************************************************************

LPT = dpr -mx

INSDIR = .

CFILES = ld.c

CFLAGS = -O -s -i


all:	ld


ld:	$(CFILES)
	$(CC) $(CFLAGS) ld.c -o ld
	cp ld $(INSDIR)/ld


listing:
	pr -n makefile $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f ld.o ld

clobber: clean
	-rm -f ld

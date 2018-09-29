#	@(#)makefile	2.2

#***************************************************************************
#                                                                          *
#		UNIX  pdp-11 Lorder (lorder) makefile			   *
#									   *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#	lorder.sh		lorder function shell script		   *
#									   *
#***************************************************************************

LPT = dpr -mx

INSDIR = .

SFILES = lorder.sh


all:	lorder


lorder:	$(SFILES)
	cp lorder.sh lorder
	chmod +x lorder
	cp lorder $(INSDIR)/lorder


listing:
	pr -n makefile $(SFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f lorder

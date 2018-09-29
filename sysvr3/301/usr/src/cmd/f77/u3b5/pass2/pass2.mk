#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)f77:u3b5/pass2/pass2.mk	1.8.1.1"
#	@(#)f77:u3b5/pass2/pass2.mk	1.8.1.1
#	3B5 or 3B15 FORTRAN COMPILER PASS2 MAKEFILE


FCOM = .
FMAC = .
PCOM = .
PMAC = .
CFLAGS = -O -c -DFLT -DSPFORT -DFORT -DFONEPASS \
	 -I$(PCOM) -I$(PMAC) -I$(FCOM) -I$(FMAC) 
LDFLAGS =
CC = cc
SIZE = size
STRIP = strip
INSTALL = install
INCLUDES = $(PCOM)/mfile2 $(PCOM)/manifest $(PMAC)/macdefs $(PMAC)/mac2defs

all:		f77pass2

f77pass2:	fort.o reader.o local2.o order.o match.o allo.o table.o comm2.o
		$(CC)  $(LDFLAGS) fort.o reader.o local2.o order.o match.o \
			allo.o table.o comm2.o -o f77pass2
		@$(SIZE) f77pass2

fort.o:		$(FCOM)/fort.c $(FMAC)/fort.h $(INCLUDES)
		$(CC) $(FFLAG) $(CFLAGS) $(FCOM)/fort.c

reader.o:	$(PCOM)/reader.c $(PCOM)/messages.h $(INCLUDES)
		$(CC) $(FFLAG) $(CFLAGS) -DNOMAIN $(PCOM)/reader.c

local2.o:	$(PMAC)/local2.c $(INCLUDES) $(ROOT)/usr/include/ctype.h
		$(CC) $(FFLAG) $(CFLAGS) $(PMAC)/local2.c

order.o:	$(PMAC)/order.c $(INCLUDES)
		$(CC) $(FFLAG) $(CFLAGS) $(PMAC)/order.c

match.o:	$(PCOM)/match.c $(INCLUDES)
		$(CC) $(FFLAG) $(CFLAGS) $(PCOM)/match.c

allo.o:		$(PCOM)/allo.c $(INCLUDES)
		$(CC) $(FFLAG) $(CFLAGS) $(PCOM)/allo.c

table.o:	$(PMAC)/table.c $(INCLUDES)
		$(CC) $(FFLAG) $(CFLAGS) $(PMAC)/table.c

comm2.o:	$(PCOM)/comm2.c $(PCOM)/common $(INCLUDES)
		$(CC) $(FFLAG) $(CFLAGS) $(PCOM)/comm2.c

install:	all
		$(STRIP) f77pass2
		$(INSTALL) -f $(ROOT)/usr/lib f77pass2

clean:
	-rm -f *.o

clobber:
	-rm -f *.o f77pass2

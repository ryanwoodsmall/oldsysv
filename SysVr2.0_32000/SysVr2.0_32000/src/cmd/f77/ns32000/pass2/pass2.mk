#	@(#)pass2.mk	1.3
#	NSC FORTRAN COMPILER PASS2 MAKEFILE


FCOM = ../../common/pass2
FMAC = .
PCOM = $(ROOT)/usr/src/cmd/cc/pcc/mip
PMAC = $(ROOT)/usr/src/cmd/cc/pcc/ns32000
CFLAGS = -O -c -DFLT -DSPFORT -DFORT \
	 -I$(PCOM) -I$(PMAC) -I$(FCOM) -I$(FMAC)
LDFLAGS =
CC = cc
SIZE = size
STRIP = strip
INSTALL = install
INCLUDES = $(PCOM)/mfile2 $(PCOM)/manifest $(PMAC)/macdefs $(PMAC)/mac2defs

all:		f77pass2

f77pass2:	fort.o reader.o local2.o order.o match.o allo.o table.o comm2.o
		$(CC) $(LDFLAGS) fort.o reader.o local2.o order.o match.o \
			allo.o table.o comm2.o -o f77pass2
		@$(SIZE) f77pass2

fort.o:		$(FCOM)/fort.c $(FMAC)/fort.h $(INCLUDES)
		$(CC) $(CFLAGS) $(FCOM)/fort.c

reader.o:	$(PCOM)/reader.c $(PCOM)/messages.h $(INCLUDES)
		$(CC) $(CFLAGS) -DNOMAIN $(PCOM)/reader.c

local2.o:	$(PMAC)/local2.c $(INCLUDES) $(ROOT)/usr/include/ctype.h
		$(CC) $(CFLAGS) $(PMAC)/local2.c

order.o:	$(PMAC)/order.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PMAC)/order.c

match.o:	$(PCOM)/match.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PCOM)/match.c

allo.o:		$(PCOM)/allo.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PCOM)/allo.c

table.o:	$(PMAC)/table.c $(INCLUDES)
		$(CC) $(CFLAGS) $(PMAC)/table.c

comm2.o:	$(PCOM)/comm2.c $(PCOM)/common $(INCLUDES)
		$(CC) $(CFLAGS) $(PCOM)/comm2.c

install:
		$(STRIP) f77pass2
		$(INSTALL) -f $(ROOT)/usr/lib f77pass2

clean:
	-rm -f *.o

clobber:
	-rm -f *.o f77pass2

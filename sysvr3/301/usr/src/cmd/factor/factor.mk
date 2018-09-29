#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)factor:factor.mk	1.6"
#	factor make file

ROOT =
OL = $(ROOT)/
INS = :
INSDIR = $(OL)usr/bin
IFLAG = -n
CFLAGS = -O $(FFLAG)
LDFLAGS = -s $(IFLAG)
SOURCE = factor.c
MAKE = make

factor:	$(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o factor factor.c -lm
	$(INS) factor $(INSDIR)

install:
	$(MAKE) -f factor.mk INS=cp OL=$(OL)

clean:
	rm -f factor.o

clobber:	clean
	  rm -f factor

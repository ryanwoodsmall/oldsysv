#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ratfor:ratfor.mk	1.6"
INS = install 
INSDIR = $(ROOT)/usr/bin
CFLAGS = -O
YACCRM=-rm
LDFLAGS=
STRIP=strip
OFILES = r0.o r1.o r2.o rio.o rlook.o rlex.o

all: ratfor

ratfor:    $(OFILES) r.g.o
	   $(CC) $(OFILES) r.g.o $(LDFLAGS) -o ratfor -ly

$(OFILES): r.h r.g.h
r.g.c r.g.h:   r.g.y
	   $(YACC) -d r.g.y
	mv y.tab.c r.g.c
	mv y.tab.h r.g.h

test:
	   rtest ratfor

install:	all
	$(STRIP) ratfor
	$(INS) -f $(INSDIR) ratfor

clean:
	   -rm -f *.o
	   $(YACCRM) -f r.g.c r.g.h

clobber:  clean
	   -rm -f ratfor

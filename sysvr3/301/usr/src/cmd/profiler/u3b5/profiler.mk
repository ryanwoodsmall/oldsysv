#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)profil-3b5:profiler.mk	1.4"
ROOT =
CC = cc
MAKE = make "CC=$(CC)"
CFLAGS = -I/include -Uvax -Du3b5 -Updp11 -O
LDFLAGS = -s
INSDIR = $(ROOT)/etc

all:	prfld prfdc prfpr prfsnap prfstat

install:
	make -f profiler.mk $(ARGS) FFLAG="$(FFLAG)" CFLAGS="$(CFLAGS)"


prfld:		prfld.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfld prfld.c $(LDLIBS)
	install -f $(INSDIR) prfld

prfdc:		prfdc.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfdc prfdc.c
	install -f $(INSDIR) prfdc

prfpr:		prfpr.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfpr prfpr.c $(LDLIBS)
	install -f $(INSDIR) prfpr

prfsnap:	prfsnap.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfsnap prfsnap.c
	install -f $(INSDIR) prfsnap

prfstat:	prfstat.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfstat prfstat.c $(LDLIBS)
	install -f $(INSDIR) prfstat

clean:
	-rm -f prfdc prfld prfpr prfsnap prfstat

clobber:	clean

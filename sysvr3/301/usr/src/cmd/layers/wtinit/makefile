#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)attwin:cmd/layers/wtinit/makefile	1.1"
#
#		Copyright 1985 AT&T
#

INC = /usr/include

all:	install

install:	wtinit
	cp wtinit /usr/lib/layersys

wtinit:	wtinit.o proto.o pcheck.o
	cc $(LDFLAGS) -o wtinit wtinit.o proto.o pcheck.o

wtinit.o:	wtinit.c
wtinit.o:	$(INC)/fcntl.h
wtinit.o:	$(INC)/termio.h
wtinit.o:	$(INC)/a.out.h
wtinit.o:	$(INC)/stdio.h
wtinit.o:	$(INC)/errno.h
wtinit.o:	$(INC)/sys/jioctl.h
wtinit.o:	proto.h
wtinit.o:	$(INC)/signal.h
wtinit.o:	$(INC)/sys/types.h
wtinit.o:	$(INC)/sys/stat.h
	cc $(CFLAGS) -c wtinit.c

proto.o:	proto.c
proto.o:	$(INC)/signal.h
proto.o:	$(INC)/stdio.h
proto.o:	proto.h
	cc $(CFLAGS) -c proto.c

pcheck.o:	pcheck.c
	cc $(CFLAGS) -c pcheck.c

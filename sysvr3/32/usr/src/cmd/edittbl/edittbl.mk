#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)edittbl:edittbl.mk	1.6.1.6"
TITLE = edittbl

MACHINE = m32
PRODUCTS = edt_data
CFLAGS = -O -I$(INC)
LDFLAGS = -s
ROOT =
INC = $(ROOT)/usr/include
STRIP = strip
DEFS = -Dm32 

all: edittbl

dgn:
	if [ ! -d $(ROOT)/dgn ];then mkdir $(ROOT)/dgn; fi;

edittbl: edittbl.c
	cc -o edittbl -I$(INC) edittbl.c
	./edittbl -t -g -s -d
	> .edt_swapp
	rm -f edittbl
	$(CC) -o edittbl $(DEFS) $(CFLAGS) $(LDFLAGS) edittbl.c $(LDLIBS)

install: dgn all
	mv edt_data $(ROOT)/dgn/edt_data
	$(CH)chmod 400 $(ROOT)/dgn/edt_data
	$(CH)chgrp 0 $(ROOT)/dgn/edt_data
	$(CH)chown 0 $(ROOT)/dgn/edt_data
	mv .edt_swapp $(ROOT)/dgn/.edt_swapp
	$(CH)chmod 400 $(ROOT)/dgn/.edt_swapp
	$(CH)chgrp 0 $(ROOT)/dgn/.edt_swapp
	$(CH)chown 0 $(ROOT)/dgn/.edt_swapp
	cp editsa $(ROOT)/etc/editsa
	$(CH)chmod 550 $(ROOT)/etc/editsa
	$(CH)chgrp 0 $(ROOT)/etc/editsa
	$(CH)chown 0 $(ROOT)/etc/editsa
	cp edittbl $(ROOT)/etc/edittbl
	$(STRIP) $(ROOT)/etc/edittbl
	$(CH)chmod 755 $(ROOT)/etc/edittbl
	$(CH)chgrp 2 $(ROOT)/etc/edittbl
	$(CH)chown 2 $(ROOT)/etc/edittbl

clobber:
	rm -f edittbl edt_data .edt_swapp

clean:

.PRECIOUS: edt_data

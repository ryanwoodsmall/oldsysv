#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)edittbl:edittbl.mk	1.6"
TITLE = edittbl

MACHINE = m32
PRODUCTS = edt_data
CFLAGS = -g
ROOT =
INCRT = $(ROOT)/usr/include
STRIP = strip
DEFS = -Du3b2 -Dm32 -Uu3b20 -Uu3b

all: edittbl

init:
	if [ ! -d $(ROOT)/init ];then mkdir $(ROOT)/init; fi;
	if [ ! -d $(ROOT)/init/dgn ];then mkdir $(ROOT)/init/dgn; fi;

edittbl: edittbl.c
	cc -o edittbl -I$(INCRT) edittbl.c
	./edittbl -t -g -s -d
	> .edt_swapp
	rm -f edittbl
	$(CC) -o edittbl $(DEFS) $(CFLAGS) edittbl.c $(LDLIBS)

install: init all
	mv edt_data $(ROOT)/init/dgn/edt_data
	$(CH)chmod 400 $(ROOT)/init/dgn/edt_data
	$(CH)chgrp 0 $(ROOT)/init/dgn/edt_data
	$(CH)chown 0 $(ROOT)/init/dgn/edt_data
	mv .edt_swapp $(ROOT)/init/dgn/.edt_swapp
	$(CH)chmod 400 $(ROOT)/init/dgn/.edt_swapp
	$(CH)chgrp 0 $(ROOT)/init/dgn/.edt_swapp
	$(CH)chown 0 $(ROOT)/init/dgn/.edt_swapp
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
	rm -f edittbl

clean:

.PRECIOUS: edt_data

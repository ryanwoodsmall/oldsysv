#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:boot/boot.mk	1.2"
STRIPOPT = -x -r

ROOT =
INCRT = $(ROOT)/usr/include
#CFLAGS = -I$(INCRT) -Uvax -Du3b2 -Uu3b5 -Updp11 -DINKERNEL $(MORECPP)
CFLAGS = -I$(INCRT) -DINKERNEL $(MORECPP)
FRC =

all:
	@for i in lboot mboot olboot;\
	do\
		cd  $$i;\
			echo "====== make -f $$i.mk \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INCRT=$(INCRT)\" \"MORECPP=$(MORECPP)\"";\
		make -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INCRT=$(INCRT)" "MORECPP=$(MORECPP)"; \
		cd .. ;\
	done;\
	wait

install:
	@for i in lboot mboot olboot;\
	do\
		cd  $$i;\
			echo "====== make -f $$i.mk install \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INCRT=$(INCRT)\" \"MORECPP=$(MORECPP)\"";\
		make -f $$i.mk install "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INCRT=$(INCRT)" "MORECPP=$(MORECPP)"; \
		cd .. ;\
	done;\
	wait

clean:
	@for i in lboot mboot olboot;\
	do\
		cd $$i;\
		make -f $$i.mk clean; \
		cd .. ;\
	done

clobber:
	@for i in lboot mboot olboot;\
	do\
		cd $$i;\
		make -f $$i.mk clobber ;\
		cd .. ;\
	done

FRC:

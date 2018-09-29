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

#ident	"@(#)kern-port:boot/boot.fast.mk	1.2.1.2"
STRIPOPT = -x -r

ROOT =
DASHO = -O
INC = $(ROOT)/usr/include
#CFLAGS = -I$(INC) -Uvax -Du3b2 -Uu3b5 -Updp11 -DINKERNEL $(MORECPP)
CFLAGS = $(DASHO)$(DASHO)  -I$(INC) -DINKERNEL $(MORECPP)
FRC =

all:
	@for i in lboot mboot olboot;\
	do\
		cd  $$i;\
			echo "====== make -f $$i.mk \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\"\"DASHO=$(DASHO)\" &";\
		make -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)"&\
		cd .. ;\
	done;\
	wait

install:
	@for i in lboot mboot olboot;\
	do\
		cd  $$i;\
			echo "====== make -f $$i.mk install \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\" &";\
		make -f $$i.mk install "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)" &\
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

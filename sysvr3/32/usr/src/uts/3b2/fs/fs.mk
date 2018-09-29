#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:fs/fs.mk	10.4.1.3"
STRIPOPT = -x -r

ROOT =
INC = $(ROOT)/usr/include
#CFLAGS = -I$(INC) -Uvax -Du3b2 -Uu3b15 -Updp11 -DINKERNEL $(MORECPP)
DASHO = -O
CFLAGS = $(DASHO) -I$(INC) -DINKERNEL $(MORECPP)
MAKE = make
FRC =

all:
	@for i in `ls`;\
	do\
		if test -d $$i;then\
		case $$i in\
		*.*)\
			;;\
		*)\
		cd  $$i;\
			echo "====== $(MAKE) -f $$i \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\" ";\
		$(MAKE) -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)"; \
		cd .. ;; \
		esac;\
		fi;\
	done

clean:
	@for i in `ls`;\
	do\
		case $$i in\
		*.*)\
			;;\
		*)\
		cd $$i;\
		$(MAKE) -f $$i.mk clean; \
		cd .. ;; \
		esac;\
	done

clobber:
	@for i in `ls`;\
	do\
		case $$i in\
		*.*)\
			;;\
		*)\
		cd $$i;\
		$(MAKE) -f $$i.mk clobber; \
		cd .. ;; \
		esac;\
	done

FRC:

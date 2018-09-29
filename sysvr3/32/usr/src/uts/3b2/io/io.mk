#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/io.mk	10.3.1.3"
STRIPOPT = -x -r

ROOT =
INC = $(ROOT)/usr/include
DASHO = -O
CFLAGS = $(DASHO) -I$(INC) -Uvax -Du3b2 -Uu3b15 -Updp11 -DINKERNEL $(MORECPP)
MAKE = make
FRC =

all:
	@for i in `ls`;\
	do\
		case $$i in\
		io*.mk)\
			;;\
		*.mk)\
			echo "====== $(MAKE) -f $$i \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\" ";\
			$(MAKE) -f $$i "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)" ; \
			;;\
		*)\
			;;\
		esac;\
	done

clean:
	@for i in `ls`;\
	do\
		case $$i in\
		io*.mk)\
			;;\
		*.mk)\
			echo "====== $(MAKE) -f $$i clean";\
			$(MAKE) -f $$i clean;;\
		*)\
			;;\
		esac;\
	done

clobber:
	@for i in `ls`;\
	do\
		case $$i in\
		io*.mk)\
			;;\
		*.mk)\
			echo "====== $(MAKE) -f $$i clobber";\
			$(MAKE) -f $$i clobber;;\
		*)\
			;;\
		esac;\
	done

FRC:

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/io.fast.mk	10.2.1.2"
STRIPOPT = -x -r

ROOT =
INC = $(ROOT)/usr/include
DASHO = -O
CFLAGS = $(DASHO) -I$(INC) -Uvax -Du3b2 -Uu3b5 -Updp11 -DINKERNEL $(MORECPP)
FRC =

all:
	@for i in `ls`;\
	do\
		case $$i in\
		io*.mk)\
			;;\
		*.mk)\
			echo "====== make -f $$i \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\" &";\
			make -f $$i "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)" &\
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
			echo "====== make -f $$i clean&";\
			make -f $$i clean;;\
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
			echo "====== make -f $$i clobber";\
			make -f $$i clobber;;\
		*)\
			;;\
		esac;\
	done

FRC:

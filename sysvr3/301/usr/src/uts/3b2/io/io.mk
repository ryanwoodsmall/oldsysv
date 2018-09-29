#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/io.mk	10.2"
STRIPOPT = -x -r

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -I$(INCRT) -Uvax -Du3b2 -Uu3b5 -Updp11 -DINKERNEL $(MORECPP)
FRC =

all:
	@for i in `ls`;\
	do\
		case $$i in\
		io*.mk)\
			;;\
		*.mk)\
			echo "====== make -f $$i \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INCRT=$(INCRT)\" \"MORECPP=$(MORECPP)\"";\
			make -f $$i "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INCRT=$(INCRT)" "MORECPP=$(MORECPP)" ; \
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
			echo "====== make -f $$i clean";\
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

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:fs/fs.mk	10.3"
STRIPOPT = -x -r

ROOT =
INCRT = $(ROOT)/usr/include
#CFLAGS = -I$(INCRT) -Uvax -Du3b2 -Uu3b5 -Updp11 -DINKERNEL $(MORECPP)
CFLAGS = -I$(INCRT) -DINKERNEL $(MORECPP)
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
			echo "====== make -f $$i \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INCRT=$(INCRT)\" \"MORECPP=$(MORECPP)\"";\
		make -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INCRT=$(INCRT)" "MORECPP=$(MORECPP)"; \
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
		make -f $$i.mk clean; \
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
		make -f $$i.mk clobber; \
		cd .. ;; \
		esac;\
	done

FRC:

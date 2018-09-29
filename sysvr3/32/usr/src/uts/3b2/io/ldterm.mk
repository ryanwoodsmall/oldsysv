#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/ldterm.mk	1.4"
ROOT =
STRIP = strip
INC = $(ROOT)/src/uts/3b2
MKBOOT = mkboot
MASTERD = ../master.d

DASHO = -O 
PFLAGS= -I$(INC) -DINKERNEL $(MORECPP)
CFLAGS= $(DASHO) -Uvax -Updp11 -Uu3b -Uu3b15 -Du3b2 -UDBUG $(PFLAGS)
FRC =

DFILES =ldterm.o 
drivers:	all

all:	LDTERM 
LDTERM:	ldterm.o $(MASTERD)/ldterm
	$(MKBOOT) -m $(MASTERD) -d . ldterm.o;

clean:
	-rm -f $(DFILES)

clobber:	clean
	-rm -f LDTERM

FRC:

#
# Header dependencies
#

ldterm.o: ldterm.c \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/types.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/region.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/ttold.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/jioctl.h \
	$(INC)/sys/ldterm.h \
	$(INC)/sys/errno.h \
	$(FRC)

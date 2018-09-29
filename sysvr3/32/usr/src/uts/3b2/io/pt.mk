#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/pt.mk	10.4"
ROOT =
STRIP = strip
INC = $(ROOT)/usr/include
MKBOOT = mkboot
MASTERD = ../master.d

DASHO = -O 
PFLAGS= -I$(INC) -DINKERNEL $(MORECPP)
CFLAGS= $(DASHO) -Uvax -Updp11 -Uu3b -Uu3b15 -Du3b2 -UDBUG $(PFLAGS)
FRC =

DFILES =ptm.o pts.o
drivers:	all

all:	PTM PTS
PTM:	ptm.o $(MASTERD)/ptm
	$(MKBOOT) -m $(MASTERD) -d . ptm.o;
PTS:	pts.o $(MASTERD)/pts
	$(MKBOOT) -m $(MASTERD) -d . pts.o;

clean:
	-rm -f $(DFILES)

clobber:	clean
	-rm -f PTM PTS

FRC:

#
# Header dependencies
#
ptm.o: ptm.c \
	$(INC)/sys/types.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5param.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/stropts.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/user.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/cmn_err.h\
	$(INC)/sys/ptms.h\
	$(FRC)

pts.o: pts.c \
	$(INC)/sys/types.h\
	$(INC)/sys/psw.h\
	$(INC)/sys/pcb.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5param.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/fs/s5dir.h\
	$(INC)/sys/stream.h\
	$(INC)/sys/stropts.h\
	$(INC)/sys/signal.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/immu.h\
	$(INC)/sys/region.h\
	$(INC)/sys/proc.h\
	$(INC)/sys/user.h\
	$(INC)/sys/debug.h\
	$(INC)/sys/cmn_err.h\
	$(INC)/sys/ptms.h\
	$(FRC)



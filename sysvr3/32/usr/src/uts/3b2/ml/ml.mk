#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:ml/ml.mk	10.6.1.2"


ROOT =
INC=$(ROOT)/usr/include
DASHO = -O
CFLAGS= $(DASHO) -I$(INC) -DINKERNEL $(MORECPP)
FRC =

SFILES = ttrap.s cswitch.s misc.s 

all:	../locore.o ../start.o ../gate.o

../locore.o:$(SFILES) $(FRC) kpcbs.o
	cat $(SFILES) | \
		sed -e 's/^#/?/' \
			-e 's/?\([ 	]*ifdef.*\)/#\1/' \
			-e 's/?\([ 	]*ifndef.*\)/#\1/' \
			-e 's/?\([ 	]*else.*\)/#\1/' \
			-e 's/?\([ 	]*endif.*\)/#\1/' \
			-e '/^?/d' > $$$$.c && \
		$(CC) -P $(MORECPP) $$$$.c && \
		mv $$$$.i locore.s && \
	$(CC) $(DASHO) -c locore.s && \
	$(LD) -r -o ../locore.o locore.o kpcbs.o && \
	rm -f locore.o locore.s $$$$.c

../start.o:$(FRC) uprt.o pstart.o cdump.o
	$(LD)  -r -o ../start.o uprt.o pstart.o cdump.o

../gate.o: gate.c \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/gate.h \
	$(INC)/sys/pcb.h \
	$(FRC)
	$(CC) $(CFLAGS) -c gate.c
	mv gate.o ../gate.o

uprt.o:uprt.s $(FRC)
	cat uprt.s | \
		sed -e 's/^#/?/' \
			-e 's/?\([ 	]*ifdef.*\)/#\1/' \
			-e 's/?\([ 	]*ifndef.*\)/#\1/' \
			-e 's/?\([ 	]*else.*\)/#\1/' \
			-e 's/?\([ 	]*endif.*\)/#\1/' \
			-e '/^?/d' > $$$$.c && \
		$(CC) -P $$$$.c && \
		mv $$$$.i $$$$.s && \
		$(CC) $(DASHO) -c $$$$.s && \
		mv $$$$.o uprt.o && \
		rm -f $$$$.?

clean:
	-rm -f a.out *.o

clobber:	clean
	-rm -f ../locore.o ../start.o ../gate.o

FRC:

#
# Header dependencies
#

cdump.o: cdump.c \
	$(INC)/sys/sbd.h \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/cdump.h \
	$(INC)/sys/todc.h \
	$(FRC)

kpcbs.o: kpcbs.c \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(FRC)

pstart.o: pstart.c \
	$(INC)/sys/types.h \
	$(INC)/sys/psw.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/sbd.h \
	$(INC)/sys/csr.h \
	$(INC)/sys/sit.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/pcb.h \
	$(INC)/sys/user.h \
	$(INC)/sys/firmware.h \
	$(INC)/sys/nvram.h \
	$(INC)/sys/var.h \
	$(INC)/sys/iobd.h \
	$(FRC)

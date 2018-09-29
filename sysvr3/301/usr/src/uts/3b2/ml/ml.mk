#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:ml/ml.mk	10.6"


ROOT =
INCRT=$(ROOT)/usr/include
DASHG =
CFLAGS= $(DASHO) $(DASHG) -I$(INCRT) -DINKERNEL $(MORECPP)
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
	$(CC) $(DASHG) -c locore.s && \
	$(LD) -r -o ../locore.o locore.o kpcbs.o && \
	rm -f locore.o locore.s $$$$.c

../start.o:$(FRC) uprt.o pstart.o cdump.o
	$(LD)  -r -o ../start.o uprt.o pstart.o cdump.o

../gate.o: gate.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/gate.h \
	$(INCRT)/sys/pcb.h \
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
		$(CC) $(DASHG) -c $$$$.s && \
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
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/cdump.h \
	$(INCRT)/sys/todc.h \
	$(FRC)

kpcbs.o: kpcbs.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(FRC)

pstart.o: pstart.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sbd.h \
	$(INCRT)/sys/csr.h \
	$(INCRT)/sys/sit.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/pcb.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/firmware.h \
	$(INCRT)/sys/nvram.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/iobd.h \
	$(FRC)

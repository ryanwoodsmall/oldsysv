#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ipc:/ipcs.mk	1.7"
#	ipcs make file

ROOT =
INCLUDE = $(ROOT)/usr/include
INSDIR = $(ROOT)/bin
LIST = lp
CFLAGS = -s -O
INS = install
FRC =

all:	ipcs ipcrm

install:	all
	$(INS) -f $(INSDIR) -m 2755 -u root -g sys ipcs
	$(INS) -f $(INSDIR) -m 755 -u root -g sys ipcrm

ipcs:
	if pdp11 ; \
	then \
		make -f ipc.mk ipcs_pdp11 ROOT=$(ROOT); \
	else \
		make -f ipc.mk ipcs_unix ROOT=$(ROOT); \
	fi
	
ipcs_unix:	$(INCLUDE)/sys/types.h\
	$(INCLUDE)/sys/ipc.h\
	$(INCLUDE)/sys/msg.h\
	$(INCLUDE)/sys/sem.h\
	$(INCLUDE)/sys/shm.h\
	$(INCLUDE)/a.out.h\
	$(INCLUDE)/fcntl.h\
	$(INCLUDE)/time.h\
	$(INCLUDE)/grp.h\
	$(INCLUDE)/pwd.h\
	$(INCLUDE)/stdio.h\
	nnlist.o \
	ipcs.c \
	$(FRC)
	$(CC) -I$(INCLUDE) $(CFLAGS) \
		-o ipcs ipcs.c nnlist.o -lld
	
ipcs_pdp11:	$(INCLUDE)/sys/types.h\
	$(INCLUDE)/sys/ipc.h\
	$(INCLUDE)/sys/msg.h\
	$(INCLUDE)/sys/sem.h\
	$(INCLUDE)/sys/shm.h\
	$(INCLUDE)/a.out.h\
	$(INCLUDE)/fcntl.h\
	$(INCLUDE)/time.h\
	$(INCLUDE)/grp.h\
	$(INCLUDE)/pwd.h\
	$(INCLUDE)/stdio.h\
	ipcs.c \
	$(FRC)
	$(CC) -I$(INCLUDE) $(CFLAGS) -o ipcs ipcs.c

ipcrm:	$(INCLUDE)/sys/types.h\
	$(INCLUDE)/sys/ipc.h\
	$(INCLUDE)/sys/msg.h\
	$(INCLUDE)/sys/sem.h\
	$(INCLUDE)/sys/shm.h\
	$(INCLUDE)/errno.h\
	$(INCLUDE)/stdio.h\
	ipcrm.c \
	$(FRC)
	$(CC) -I$(INCLUDE) $(CFLAGS) -o ipcrm ipcrm.c

nnlist.o:	$(INCLUDE)/stdio.h \
	$(INCLUDE)/filehdr.h\
	$(INCLUDE)/syms.h\
	$(INCLUDE)/ldfcn.h\
	nnlist.c \
	$(FRC)
	$(CC) -I$(INCLUDE) $(CFLAGS) -c nnlist.c
	
listing:
	pr ipc.mk ipcs.c ipcrm.c nnlist.c | $(LIST)

clean:
	-rm -f *.o

clobber: clean
	rm -f ipcs ipcrm


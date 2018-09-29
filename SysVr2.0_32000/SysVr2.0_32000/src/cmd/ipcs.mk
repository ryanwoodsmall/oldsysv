#	@(#)ipcs.mk	1.5
#	ipcs make file

ROOT =
INCLUDE = $(ROOT)/usr/include
INSDIR = $(ROOT)/bin
LIST = lp
CFLAGS = -s -O
INS=:

all:	ipcs

ipcs:	$(INCLUDE)/sys/types.h\
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
	ipcs.c
	$(CC) -I$(INCLUDE) $(CFLAGS) -o ipcs ipcs.c
	$(INS) $(INSDIR) -m 2755 -u root -g sys ipcs

install:
	$(MAKE) -f ipcs.mk INS="install -f"

listing:
	pr ipcs.mk ipcs.c | $(LIST)

clean:
	-rm -f *.o

clobber: clean
	rm -f ipcs

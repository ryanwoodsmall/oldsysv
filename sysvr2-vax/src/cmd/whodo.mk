#	@(#)whodo.mk	1.5
#       whodo make file

ROOT =
INCLUDE = $(ROOT)/usr/include
INSDIR = $(ROOT)/etc
CFLAGS = -s -O
INS=/etc/install

all:	whodo

whodo:
	$(CC) -I$(INCLUDE) $(CFLAGS) -o whodo whodo.c

install:	all
	$(INS) -f $(INSDIR) -m 2755 -g sys whodo

clean:
	-rm -f *.o

clobber:	clean
	rm -f whodo

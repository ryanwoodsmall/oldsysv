#	@(#)fuser.mk	1.1
#       fuser make file

ROOT =
INCLUDE = $(ROOT)/usr/include
INSDIR = $(ROOT)/etc
CFLAGS = -s -O
INS=/etc/install

all:	fuser

fuser:
	$(CC) -I$(INCLUDE) $(CFLAGS) -o fuser fuser.c

install:	all
	$(INS) -f $(INSDIR) -m 2755 -g sys fuser

clean:
	-rm -f *.o

clobber:	clean
	rm -f fuser

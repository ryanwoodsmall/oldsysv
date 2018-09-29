#	@(#)rmdir.mk	1.3
#	rmdir make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s
INS=:

rmdir:
	$(CC) $(LDFLAGS) -o rmdir rmdir.c
	$(INS) $(INSDIR) -m 4755 -u root -g sys rmdir

all:	install clobber

install:
	$(MAKE) -f rmdir.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f rmdir

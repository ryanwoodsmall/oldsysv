#	@(#)ct.mk	1.2
#	ct make file
ROOT=
INSDIR = /usr/bin
LDFLAGS = -O -s
INS=:

ct:
	$(CC) $(LDFLAGS) -o ct ct.c
	$(INS) $(INSDIR) -m 4755 -u root -g sys ct

all:	install clobber

install:
	$(MAKE) -f ct.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f ct

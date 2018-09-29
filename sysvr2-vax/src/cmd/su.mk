#	@(#)su.mk	1.2
#	su make file
ROOT=
INSDIR = /bin
LDFLAGS = -O -s
INS=:

su:
	$(CC) $(LDFLAGS) -o su su.c
	$(INS) $(INSDIR) -m 4755 -u root -g sys su

all:	install clobber

install:
	$(MAKE) -f su.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f su

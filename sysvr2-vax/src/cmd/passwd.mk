#	@(#)passwd.mk	1.2
#	passwd make file
ROOT=
INSDIR = /bin
LDFLAGS = -O -s
INS=:

passwd:
	$(CC) $(LDFLAGS) -o passwd passwd.c
	$(INS) $(INSDIR) -m 6755 -u root -g sys passwd

all:	install clobber

install:
	$(MAKE) -f passwd.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f passwd

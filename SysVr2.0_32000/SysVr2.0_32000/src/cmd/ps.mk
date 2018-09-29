#	@(#)ps.mk	1.2
#	ps make file
ROOT=
INSDIR = /bin
LDFLAGS = -O -s
INS=:

ps:
	$(CC) $(LDFLAGS) -o ps ps.c
	$(INS) $(INSDIR) -m 2755 -u root -g sys ps

all:	install clobber

install:
	$(MAKE) -f ps.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f ps

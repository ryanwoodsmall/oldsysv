#	@(#)mkdir.mk	1.2
#	mkdir make file
ROOT=
INSDIR = /bin
LDFLAGS = -O -s
INS=:

mkdir:
	$(CC) $(LDFLAGS) -o mkdir mkdir.c
	$(INS) $(INSDIR) -m 4755 -u root -g sys mkdir

all:	install clobber

install:
	$(MAKE) -f mkdir.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f mkdir

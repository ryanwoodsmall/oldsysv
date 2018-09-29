#	@(#)c2.mk	1.4
#			makefile for vax c2 optimizer
#
TESTDIR	= .
INS	= /etc/install
DESTDIR	=
FRC	=
CFLAGS	= -O
LDFLAGS	= -s
FIXDIR	= ../pcc/vax

all:	c2

c2:	c20.o c21.o c22.o c23.o nargs.o
	$(CC) $(LDFLAGS) -o $(TESTDIR)/c2 c2?.o nargs.o

mon.o: /usr/lib/mon.c
	$(CC) $(CFLAGS) -c -p -DMASTER='"/usr/lib/c2.monsum"' \
		-DINCREMENT='"/tmp/c2XXXXX"' /usr/lib/mon.c
	
c20.o:	c20.c c2.h

c21.o:	c21.c c2.h

c22.o:  c22.c c2.h
	$(CC) $(CFLAGS) -S c22.c
	-chmod +x $(FIXDIR)/:rofix
	$(FIXDIR)/:rofix c22.s
	$(CC) $(CFLAGS) -c c22.s

c23.o:	c23.c c2.h

nargs.o: nargs.s
	$(CC) $(CFLAGS) -c nargs.s

clean :
	rm -f *.o c22.s

clobber: clean
	rm -f $(TESTDIR)/c2

install : c2
	$(INS) -f $(DESTDIR)/lib $(TESTDIR)/c2

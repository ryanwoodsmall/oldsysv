#	@(#)lib2A.mk	1.2
all:	Aconf.o Ainit.o
	ar rvc lib2A.a Aconf.o Ainit.o

install: all
	/etc/install -f /usr/lib lib2A.a

Aconf.o:
	cc -O -c Aconf.c

Ainit.o:
	cc -O -c Ainit.c

clean:
	rm -f Aconf.o Ainit.o

clobber: clean
	rm -f lib2A.a

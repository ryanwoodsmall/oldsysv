#	@(#)liby.mk	1.2
ROOT=
ULIBDIR=$(ROOT)/usr/lib
LORDER=lorder

CC=cc
CFLAGS=

LIBOBJS=libmai.o libzer.o

all:     $(ULIBDIR)/liby.a

$(ULIBDIR)/liby.a: $(LIBOBJS)
	ar rv tmplib.a `$(LORDER) *.o | tsort`;

libmai.o:	libmai.c
		$(CC) -c $(CFLAGS) libmai.c

libzer.o:	libzer.c
		$(CC) -c $(CFLAGS) libzer.c

install:  all
	cp tmplib.a $(ULIBDIR)/liby.a;

clean:
	-rm -f *.o

clobber:	clean
	-rm -f tmplib.a 

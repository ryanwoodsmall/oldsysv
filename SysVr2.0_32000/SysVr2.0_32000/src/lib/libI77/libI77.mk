#	@(#)libI77.mk	1.10
#	LIBI77 MAKEFILE

CC = cc
AR = ar
STRIP = strip
CFLAGS = -O
INS = install
INSDIR = $(ROOT)/usr/lib

OBJ =	Version.o backspace.o dfe.o due.o iio.o inquire.o rewind.o rsfe.o \
	rdfmt.o sue.o uio.o wsfe.o sfe.o fmt.o nio.o lio.o lread.o open.o \
	close.o util.o endfile.o wrtfmt.o err.o fmtlib.o ecvt.o ltostr.o

all:		libI77.a

libI77.a:	$(OBJ)
		$(AR) r libI77.a $?
	if pdp11; then \
		$(STRIP) libI77.a; \
	else \
		$(STRIP) -r libI77.a; \
		$(AR) ts libI77.a; \
	fi;

lio.o:	lio.h

lread.o: lio.h

.c.o:
	$(CC) -c $(CFLAGS) $<

install:	all
	$(INS) -f $(INSDIR) libI77.a

clean:
	-rm -f $(OBJ)

clobber:	clean
	-rm -f libI77.a

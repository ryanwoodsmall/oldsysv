#	@(#)libI77.mk	1.6
TESTDIR = .
LIBNAME = $(TESTDIR)/libI77.a
INS = /etc/install
INSDIR = /usr/lib
FRC =
CFLAGS = -O
FILES =\
	$(LIBNAME)(Version.o)\
	$(LIBNAME)(backspace.o)\
	$(LIBNAME)(dfe.o)\
	$(LIBNAME)(due.o)\
	$(LIBNAME)(iio.o)\
	$(LIBNAME)(inquire.o)\
	$(LIBNAME)(rewind.o)\
	$(LIBNAME)(rsfe.o)\
	$(LIBNAME)(rdfmt.o)\
	$(LIBNAME)(sue.o)\
	$(LIBNAME)(uio.o)\
	$(LIBNAME)(wsfe.o)\
	$(LIBNAME)(sfe.o)\
	$(LIBNAME)(fmt.o)\
	$(LIBNAME)(nio.o)\
	$(LIBNAME)(lio.o)\
	$(LIBNAME)(lread.o)\
	$(LIBNAME)(open.o)\
	$(LIBNAME)(close.o)\
	$(LIBNAME)(util.o)\
	$(LIBNAME)(endfile.o)\
	$(LIBNAME)(wrtfmt.o)\
	$(LIBNAME)(err.o)\
	$(LIBNAME)(fmtlib.o)

all:	$(LIBNAME)

$(LIBNAME):	$(FILES) /usr/include/stdio.h fio.h
	if pdp11 ; then strip $(LIBNAME) ; else strip -r $(LIBNAME) ; ar ts $(LIBNAME) ; fi

$(LIBNAME)(lio.o): lio.h

$(LIBNAME)(lread.o): lio.h

.c.a:
	$(CC) -c $(CFLAGS) $*.c
	ar rv $@ $*.o
	-rm -f $*.o

install:	all
	$(INS) -n $(INSDIR) $(LIBNAME)

clean:
	-rm -f $*.o

clobber:	clean
	-rm -f $(LIBNAME)

FRC:

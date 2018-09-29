#	@(#)make.mk	1.7

# The rules.c file can be modified locally for people who still like
#	things like fortran.

TESTDIR = .
YACCRM = rm -f
LDFLAGS = -i -n
INS = install -o
INSDIR = $(ROOT)/bin
LIBS = -lld

OBJECTS =  \
	main.o \
	doname.o \
	misc.o \
	files.o \
	rules.o \
	dosys.o \
	gram.o \
	dyndep.o \
	prtmem.o

CFLAGS = -O

all:  make
	@echo MAKE is up to date.

make:  $(OBJECTS)
	if pdp11 ; then \
		$(CC) -o $(TESTDIR)/make $(LDFLAGS) $(OBJECTS) ; \
	else \
		$(CC) -o $(TESTDIR)/make $(LDFLAGS) $(OBJECTS) $(LIBS) ; \
	fi

gram.c:	gram.y

gram.o: gram.c

$(OBJECTS):  defs

install: all
	$(INS) -n $(INSDIR) $(TESTDIR)/make

clean:
	-rm -f *.o
	$(YACCRM) gram.c

clobber: clean
	-rm -f $(TESTDIR)/make

.DEFAULT:
	$(GET) $(GFLAGS) -p s.$< > $<

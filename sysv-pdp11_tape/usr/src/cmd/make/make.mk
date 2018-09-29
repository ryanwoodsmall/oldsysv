#	@(#)/usr/src/cmd/make/make.mk	3.5

#	@(#)make.mk	3.1

# Description file for the Make command
# To build the Columbus make just type "make -f make.mk"
# For systems without the getu() system call type "make -f make.mk GETU="
# For pwb use
#	make -f make.mk GETU= RULES=pwbrules CFLAGS="-O -DPWB" LIBES=-lpw
# The rules.c file can be modified locally for people who still like
#	things like fortran.
# If the markfile is not present the compilation can be turned of with
#	"make -f make.mk MARKFILE="
# For people interested in finding out how the markfile is generated
# 	contact E. G. Bradford.

TESTDIR = .
YACCRM = rm -f
IFLAG = -i
RC =
INS = /etc/install -o
LIBES = -lld
INSDIR = /bin

#	There is an internal rule (rules.c) which can make
#	and object file "markfile.o" from "markfile" if the
#	"markfile" has a what(1) string in it.
#	Disable the $(MARKFILE) macro if the internal: rule
#	doesn't work on your system.

#	Turn off the GETU variable if you system does not have the
#	getu system call as documented in the CB unix manual.
GETU	=

#	RULES is defined to be the Columbus rules. It can be defined to
#	be the pwb rules by typing:
#		make -f make.mk RULES=pwbrules
#
RULES	= rules

#	For PWB systems the PWB define symbol must be in the CFLAGS macro.
#	Thus for pwb systems:
#	    make -f make.mk CFLAGS="-O -DPWB" GETU= RULES=pwbrules LIBES=-lpw
#	will make a version of "make" for PWB systems.


OBJECTS =  \
	main.o \
	doname.o \
	misc.o \
	files.o \
	$(RULES).o \
	dosys.o \
	gram.o \
	dyndep.o \
	prtmem.o

CFLAGS = -O $(GETU)

all:  make
	@echo MAKE is built.

make:  $(OBJECTS) $(RC)
	-if pdp11; \
	then \
		$(CC) $(IFLAG) $(LDFLAGS) $(OBJECTS); \
	else \
		$(CC) $(IFLAG) $(LDFLAGS) $(OBJECTS) $(LIBES); \
	fi

	mv a.out $(TESTDIR)/make
	@size make

gram.c:	gram.y

gram.o: gram.c

$(OBJECTS):  defs


v6:
	make -f make.mk CC=occ LIBES="-lcb -li1 -l7 -lS"

test:
	make -f test.mk

install: all
	$(INS) -n $(INSDIR) $(TESTDIR)/make

clean cleanup:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/make
	$(YACCRM) gram.c

print:
	list="defs main.c doname.c dosys.c files.c $(RULES).c misc.c dyndep.c prtmem.c gram.y make.mk test.mk test.mk?" \
	pr -o1 -w120 -l66 $list

RC:


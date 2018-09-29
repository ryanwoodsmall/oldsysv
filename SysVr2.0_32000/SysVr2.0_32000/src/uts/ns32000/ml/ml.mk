# @(#)ml.mk	1.5
LIBNAME = ../lib0
INCRT = /usr/include
CFLAGS = -O -I$(INCRT)
FRC =

FILES =\
	$(LIBNAME)(dcdump.o)\
	$(LIBNAME)(tcdump.o)

MFILES = param.m

SFILES = start.s dump.s misc.s lmr.s end.s copy.s userio.s trap.s cswitch.s math.s page.s

all:	$(LIBNAME) ../locore.o

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES) $(FRC)

../locore.o:	$(SFILES) $(MFILES) $(FRC)
	cat $(MFILES) $(SFILES) > locore.s
	$(AS) -o ../locore.o locore.s
	-rm -f locore.s

clean:
	-rm -f locore.s *.o

clobber:	clean
	-rm -f $(LIBNAME) ../locore.o

$(LIBNAME)(tcdump.o):\
	$(INCRT)/sys/types.h\
	$(FRC)

$(LIBNAME)(dcdump.o):\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/disk.h\
	$(INCRT)/sys/dcu.h\
	$(FRC)

FRC:

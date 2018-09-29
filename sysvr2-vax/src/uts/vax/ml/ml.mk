# @(#)ml.mk	6.3
LIBNAME = ../lib0
INCRT = /usr/include
CFLAGS = -O -I$(INCRT) -W2,-y
FRC =

FILES =\
	$(LIBNAME)(gddump.o) \
	$(LIBNAME)(gtdump.o) \
	$(LIBNAME)(htdump.o) \
	$(LIBNAME)(hudump.o) \
	$(LIBNAME)(tsdump.o)

IFILES =\
	$(LIBNAME)(gddump.o) \
	$(LIBNAME)(gtdump.o) \
	$(LIBNAME)(htdump.o) \
	$(LIBNAME)(hudump.o) \
	$(LIBNAME)(tsdump.o)

MFILES = mtpr.m page.m psl.m rpb.m trap.m uba.m

SFILES = rpb.s dump.s scb.s trap.s paging.s power.s random.s sbi.s  \
	unibus.s start.s cswitch.s userio.s copy.s math.s misc.s end.s

all:	$(LIBNAME) ../locore.o

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES) $(FRC)
	-chown bin $(LIBNAME)
	-chgrp bin $(LIBNAME)
	-chmod 664 $(LIBNAME)

../locore.o:	$(SFILES) $(MFILES) $(FRC)
	cat $(MFILES) $(SFILES) > locore.s
	$(AS) -o ../locore.o locore.s
	-chown bin ../locore.o
	-chgrp bin ../locore.o
	-chmod 664 ../locore.o
	-rm -f locore.s

clean:
	-rm -f locore.s *.o

clobber:	clean
	-rm -f $(LIBNAME) ../locore.o

FRC:

#/*	@(#)pcc.mk	1.6	*/
TESTDIR	= .
INS	=
FRC	=
CFLAGS	= -O
DFLAG	= -DBUG4
IFLAG	= -i
INSTALL	=
M	= ../mip

all: itest comp

itest:
	@if [ x$(IFLAG) != x-i ] ; then echo NO ID SPACE ; exit 1 ; else exit 0 ; fi
head:  comp ;
comp: cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o local.o reader.o local2.o order.o match.o allo.o comm1.o table.o messages.o fltused.o
	${CC} $(CFLAGS) $(DFLAG) -i cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o local.o  reader.o local2.o order.o match.o allo.o comm1.o table.o messages.o fltused.o
	mv a.out comp
trees.o: $M/manifest macdefs $M/mfile1 $M/messages.h $M/trees.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/trees.c
optim.o: $M/manifest macdefs $M/mfile1 $M/optim.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/optim.c
pftn.o: $M/manifest macdefs $M/mfile1 $M/messages.h $M/pftn.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/pftn.c
code.o: $M/manifest macdefs $M/mfile1
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. code.c
local.o: $M/manifest macdefs $M/mfile1
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. local.c
scan.o: $M/manifest macdefs $M/mfile1 $M/messages.h $M/scan.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/scan.c
xdefs.o: $M/manifest $M/mfile1 macdefs $M/xdefs.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/xdefs.c
cgram.o: $M/manifest $M/mfile1 $M/messages.h macdefs $M/cgram.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/cgram.c
$M/cgram.c: $M/cgram.y
	yacc $M/cgram.y
	mv y.tab.c $M/cgram.c
comm1.o: $M/manifest $M/mfile1 $M/common macdefs $M/comm1.c
	${CC} -c $(CFLAGS) $(DFLAG) -I. -I$M $M/comm1.c
table.o: $M/manifest $M/mfile2 mac2defs macdefs table.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. table.c
reader.o: $M/manifest $M/mfile2 $M/messages.h mac2defs macdefs $M/reader.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/reader.c
local2.o: $M/manifest $M/mfile2 mac2defs macdefs
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. local2.c
order.o: $M/manifest $M/mfile2 mac2defs macdefs
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. order.c
match.o: $M/manifest $M/mfile2 mac2defs macdefs $M/match.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/match.c
allo.o: $M/manifest $M/mfile2 mac2defs macdefs $M/allo.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/allo.c
messages.o: $M/messages.h $M/messages.c
	${CC} -c $(CFLAGS) $(DFLAG) -I$M -I. $M/messages.c
fltused.o: fltused.c
	pcc -c fltused.c
clean:
	-rm -f *.o $M/cgram.c
clobber:	clean
	-rm -f comp

lintall:
	lint -hpv -I. -I$M  $M/cgram.c $M/xdefs.c $M/scan.c $M/pftn.c $M/trees.c $M/optim.c code.c local.c $M/reader.c local2.c order.c $M/match.c $M/allo.c $M/comm1.c table.c $M/messages.c

install:
	cp comp $(INSTALL)

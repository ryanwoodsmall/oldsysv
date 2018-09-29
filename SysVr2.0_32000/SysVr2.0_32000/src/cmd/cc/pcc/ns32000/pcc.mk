#	@(#)pcc.mk	1.8

TESTDIR	= .
INS	= /etc/install
INSDIR	=
CC	= cc
YACC    = yacc
CFLAGS	= -O -DFLEXNAMES
MFLAGS  =
LDFLAGS	= -s
INSTALL	= nccom
M	= ../mip

all: comp

objects: cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o \
		local.o reader.o local2.o order.o match.o allo.o \
		comm1.o table.o messages.o

comp: rodata.o cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o \
		local.o reader.o local2.o order.o match.o allo.o \
		comm1.o table.o messages.o
	$(CC) $(LDFLAGS) -i rodata.o cgram.o xdefs.o scan.o pftn.o trees.o \
		optim.o code.o local.o reader.o local2.o order.o \
		match.o allo.o comm1.o table.o messages.o -o $(TESTDIR)/comp

trees.o: $M/manifest macdefs $M/mfile1 $M/messages.h $M/trees.c
	$(CC) -c $(CFLAGS) -I$M -I.  $M/trees.c

optim.o: $M/manifest macdefs $M/mfile1 $M/optim.c
	$(CC) -c $(CFLAGS) -I$M -I. $M/optim.c

pftn.o: $M/manifest macdefs $M/mfile1 $M/messages.h $M/pftn.c
	$(CC) -c $(CFLAGS) -I$M -I. $M/pftn.c

code.o: $M/manifest macdefs $M/mfile1 $M/messages.h
	$(CC) -c $(CFLAGS) -I$M -I. code.c

local.o: $M/manifest macdefs $M/mfile1
	$(CC) -c $(CFLAGS) -I$M -I. local.c

scan.o: $M/manifest macdefs $M/mfile1 $M/messages.h $M/scan.c
	$(CC) -c $(CFLAGS) -I$M -I. $M/scan.c

xdefs.o: $M/manifest $M/mfile1 macdefs $M/xdefs.c
	$(CC) -c $(CFLAGS) -I$M -I. $M/xdefs.c

cgram.o: $M/manifest $M/mfile1 macdefs $M/messages.h $M/cgram.c
	$(CC) -c $(CFLAGS) -DYYMAXDEPTH=300 -I. -I$M $M/cgram.c

rodata.o $M/cgram.c: $M/cgram.y
	$(YACC) $M/cgram.y
	rm -f rodata.c
	sh ./:yyfix >rodata.c
	mv y.tab.c $M/cgram.c
	$(CC) $(CFLAGS) -S rodata.c
	-if vax || ns32000; then\
		sh ./:rofix rodata.s;\
	fi
	$(CC) $(CFLAGS) -c rodata.s
#	rm -f rodata.s rodata.c

comm1.o: $M/manifest $M/mfile1 $M/common macdefs $M/comm1.c
	$(CC) -c $(CFLAGS) -I. -I$M $M/comm1.c

table.o: $M/manifest $M/mfile2 mac2defs macdefs table.c
	$(CC) $(CFLAGS) -S -I$M -I. table.c
	if vax || ns32000; then\
		sh ./:rofix table.s;\
	fi
	$(CC) $(CFLAGS) -c table.s
	rm -f table.s

messages.o: $M/messages.c $M/messages.h
	$(CC) $(CFLAGS) -S -I$M -I. $M/messages.c
	if vax || ns32000; then\
		sh ./:rofix messages.s;\
	fi
	$(CC) $(CFLAGS) -c messages.s
	rm -f messages.s

reader.o: $M/manifest $M/mfile2 mac2defs macdefs $M/messages.h $M/reader.c
	$(CC) -c $(CFLAGS) -I$M -I. $M/reader.c

local2.o: $M/manifest $M/mfile2 mac2defs macdefs
	$(CC) -c $(CFLAGS) -I$M -I. local2.c

order.o: $M/manifest $M/mfile2 mac2defs macdefs
	$(CC) -c $(CFLAGS) -I$M -I. order.c

match.o: $M/manifest $M/mfile2 mac2defs macdefs $M/match.c
	$(CC) -c $(MFLAGS) $(CFLAGS) -I$M -I. $M/match.c

allo.o: $M/manifest $M/mfile2 mac2defs macdefs $M/allo.c
	$(CC) -c $(CFLAGS) -I$M -I. $M/allo.c

shrink:
	rm *.o comp

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/comp $M/cgram.c

lintall:
	lint -hpv -I. -I$M  $M/cgram.c $M/xdefs.c $M/scan.c $M/pftn.c \
		$M/trees.c $M/optim.c code.c local.c $M/reader \
		local2.c order.c $M/match.c $M/allo.c $M/comm1.c table.c

install:	all
	ln $(TESTDIR)/comp $(INSTALL)
	rm -f /lib/nccom
	-cp /lib/ccom /lib/nccom
	$(INS) -i $(INSTALL) $(INSDIR)/lib
	rm $(INSTALL)

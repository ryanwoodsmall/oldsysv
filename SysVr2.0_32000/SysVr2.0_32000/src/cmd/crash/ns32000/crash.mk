# @(#)crash.mk	1.8
TESTDIR = .
FRC =
INS = /etc/install
INSDIR = /etc
CFLAGS = -O
LDFLAGS =  -s
CC = cc
SYS =

OFILES = buf.o callout.o file.o inode.o lck.o main.o $(SYS)misc.o mount.o proc.o\
		$(SYS)symtab.o stat.o text.o tty.o u.o sysvad.o region.o

all:	stest
stest:
	-if vax ; then make -f crash.mk comp SYS=VAX ; \
	elif pdp11 ; then make -f crash.mk comp SYS=PDP ; \
	elif ns32000 ; then make -f crash.mk comp SYS=NSC ; fi

comp:	$(OFILES) cmd.h crash.h 
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o $(TESTDIR)/crash $(OFILES)

install: stest
	$(INS) -n /etc $(TESTDIR)/crash $(INSDIR)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/crash

FRC:

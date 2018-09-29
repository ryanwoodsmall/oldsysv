#	mpl.mk 1.2 of 12/4/82
#	@(#)mpl.mk	1.2
YACCRM=-rm
TESTDIR = .
FRC =
INS = /etc/install -n /usr/lib/vpm
INSDIR =
CFLAGS = -O

all:		mpl 

mpl:		mpl.c mpl.yl
		$(CC) $(LDFLAGS) -s -o $(TESTDIR)/mpl mpl.c -ly -ll

mpl.c:		mpl.y $(FRC)
		$(YACC) mpl.y
		mv y.tab.c mpl.c

mpl.yl:		mpl.l $(FRC)
		lex mpl.l
		mv lex.yy.c mpl.yl

install:	all
		$(INS) $(TESTDIR)/mpl $(INSDIR)

clean:
		-rm -f *.o

clobber:	clean
		-rm -f $(TESTDIR)/mpl
		$(YACCRM) -f $(TESTDIR)/mpl.yl $(TESTDIR)/mpl.c

#	@(#)kasb.mk	1.1
YACCRM=-rm
TESTDIR = .
FRC =
INS = /etc/install -n /bin
INSDIR =
CFLAGS = -O $(CCFLAGS)

all:	kasb

kasb:	kas0.o kas1.o kas2.o kas3.o
	$(CC) $(CFLAGS) -s -o $(TESTDIR)/kasb kas0.o kas1.o kas2.o kas3.o -ll
kas0.o:	kas.h kas0.c kas0.yl $(FRC)
kas1.o:	kas.h kas1.c $(FRC)
kas2.o: kas.h kas2.c kas.yh $(FRC)
kas3.o: kas.h kas3.c $(FRC)
kas0.c:	kas0.y $(FRC)
	$(YACC) -d kas0.y
	mv y.tab.c kas0.c
	mv y.tab.h kas.yh
kas0.yl: kas0.l $(FRC)
	lex kas0.l
	mv lex.yy.c kas0.yl

install:	all
	$(INS)	$(TESTDIR)/kasb $(INSDIR)

clean:
	-rm *.o

clobber:	clean
	-rm $(TESTDIR)/kasb
	$(YACCRM) -f $(TESTDIR)/kas0.yl $(TESTDIR)/kas.yh $(TESTDIR)/kas0.c

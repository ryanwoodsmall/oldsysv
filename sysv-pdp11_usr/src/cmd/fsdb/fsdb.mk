#	@(#)fsdb.mk	1.2
TESTDIR = .
INSDIR = /etc
INC = /usr/include
INS = :
CFLAGS = -s -O -D
FRC =

all: fsdb
fsdb::\
	/usr/include/stdio.h\
	$(INC)/sys/param.h\
	/usr/include/signal.h\
	$(INC)/sys/types.h\
	$(INC)/sys/sysmacros.h\
	$(INC)/sys/filsys.h\
	$(INC)/sys/dir.h\
	$(INC)/sys/ino.h\
	fsdb.c
	-if pdp11; \
	then \
		$(CC) $(CFLAGS)FsTYPE=1 -I$(INC) -o $(TESTDIR)/fsdb fsdb.c ;\
	else \
		$(CC) $(CFLAGS)FsTYPE=2 -I$(INC) -o $(TESTDIR)/fsdb fsdb.c ;\
		$(CC) $(CFLAGS)FsTYPE=1 -I$(INC) -o $(TESTDIR)/fsdb1b fsdb.c; \
	fi

fsdb::
	-if pdp11; \
	then \
		$(INS) -n $(INSDIR) $(TESTDIR)/fsdb ; \
	else \
		$(INS) -n $(INSDIR) $(TESTDIR)/fsdb ; \
		$(INS) -n $(INSDIR) $(TESTDIR)/fsdb1b ; \
	fi

install:
	make -f fsdb.mk INS=/etc/install FRC="$(FRC)" TESTDIR="$(TESTDIR)" \
		INSDIR="$(INSDIR)" INCRT="$(INCRT)" CFLAGS="$(CFLAGS)"

clean:

clobber : clean
	rm -f $(TESTDIR)/fsdb $(TESTDIR)/fsdb1b

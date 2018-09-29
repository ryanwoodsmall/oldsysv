#	@(#)dcopy.mk	1.2
ROOT=
TESTDIR = .
INSDIR = $(ROOT)/etc
INCRT = $(ROOT)/usr/include
INS = /etc/install
CFLAGS = -O
FRC =

all: dcopy dcopy1b
dcopy::\
	$(INCRT)/stdio.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	dcopy.c\
	$(FRC)
	$(CC) $(CFLAGS) -DFsTYPE=2 -o $(TESTDIR)/dcopy dcopy.c

dcopy::
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy

dcopy1b::\
	$(INCRT)/stdio.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	dcopy.c\
	$(FRC)
	$(CC) $(CFLAGS) -DFsTYPE=1 -o $(TESTDIR)/dcopy1b dcopy.c

dcopy1b::
	-if pdp11 ; then ln $(TESTDIR)/dcopy1b $(TESTDIR)/dcopy ; \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy ; \
	else $(INS) -f $(INSDIR) $(TESTDIR)/dcopy1b ; fi

FRC :

install:
	-if pdp11 ; then make -f dcopy.mk dcopy1b INS=/etc/install\
		TESTDIR="$(TESTDIR)" INSDIR="$(INSDIR)"\
		INCRT="$(INCRT)" CFLAGS="$(CFLAGS)" ; \
	else make -f dcopy.mk all INS=/etc/install FRC=$(FRC)\
		TESTDIR="$(TESTDIR)" INSDIR="$(INSDIR)"\
		INCRT="$(INCRT)" CFLAGS="$(CFLAGS)" ; fi

clean:
	rm -f *.o

clobber : clean
	rm -f $(TESTDIR)/dcopy $(TESTDIR)/dcopy1b

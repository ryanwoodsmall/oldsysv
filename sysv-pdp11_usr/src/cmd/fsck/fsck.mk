#	@(#)fsck.mk	1.1
TESTDIR = .
INSDIR = /etc
INCRT = /usr/include
INS = :
CFLAGS = -O
FRC =

all: fsck fsck1b
fsck::\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/stat.h\
	fsck.c\
	$(FRC)
	$(CC) $(CFLAGS) -DFsTYPE=2 -o $(TESTDIR)/fsck fsck.c

fsck::
	$(INS) -n $(INSDIR) $(TESTDIR)/fsck

fsck1b::\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/stat.h\
	fsck.c\
	$(FRC)
	$(CC) $(CFLAGS) -DFsTYPE=1 -o $(TESTDIR)/fsck1b fsck.c

fsck1b::
	-if pdp11 ; then ln $(TESTDIR)/fsck1b $(TESTDIR)/fsck ; \
	$(INS) -n $(INSDIR) $(TESTDIR)/fsck ; \
	else $(INS) -n $(INSDIR) $(TESTDIR)/fsck1b ; fi

FRC :

install:
	-if pdp11 ; then make -f fsck.mk fsck1b INS=/etc/install\
		TESTDIR="$(TESTDIR)" INSDIR="$(INSDIR)"\
		INCRT="$(INCRT)" CFLAGS="$(CFLAGS)" ; \
	else make -f fsck.mk all INS=/etc/install FRC=$(FRC)\
		TESTDIR="$(TESTDIR)" INSDIR="$(INSDIR)"\
		INCRT="$(INCRT)" CFLAGS="$(CFLAGS)" ; fi

clean:
	rm -f *.o

clobber : clean
	rm -f $(TESTDIR)/fsck $(TESTDIR)/fsck1b

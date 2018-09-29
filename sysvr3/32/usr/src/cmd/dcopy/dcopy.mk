#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dcopy:dcopy.mk	1.12"
ROOT=
TESTDIR = .
INSDIR = $(ROOT)/etc
INCRT = $(ROOT)/usr/include
INS = install
CFLAGS = -O -I$(INCRT)
LDFLAGS = -s
FRC =

all:
	-if u3b2; then make -f dcopy.mk dcopy512 dcopy1K dcopy2K; \
	elif u3b15; then make -f dcopy.mk dcopy1b dcopy2b dcopy; \
	else make -f dcopy.mk dcopy1b dcopy; fi

dcopy512 dcopy1b::\
	$(INCRT)/stdio.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/fs/s5filsys.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	dcopy.c\
	$(FRC)
	-if u3b2; \
	then \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=1 -o $(TESTDIR)/dcopy512 dcopy.c $(LDLIBS);\
	else \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=1 -o $(TESTDIR)/dcopy1b dcopy.c $(LDLIBS);\
	fi

dcopy1K dcopy::\
	$(INCRT)/stdio.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/fs/s5filsys.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	dcopy.c\
	$(FRC)
	-if u3b2; \
	then \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=2 -o $(TESTDIR)/dcopy1K dcopy.c $(LDLIBS);\
	else \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=2 -o $(TESTDIR)/dcopy dcopy.c $(LDLIBS);\
	fi

dcopy2K dcopy2b::\
	$(INCRT)/stdio.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/fcntl.h\
	$(INCRT)/sys/fs/s5filsys.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	dcopy.c\
	$(FRC)
	-if u3b2; \
	then \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=4 -o $(TESTDIR)/dcopy2K dcopy.c $(LDLIBS);\
	elif u3b15; \
	then \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=4 -o $(TESTDIR)/dcopy2b dcopy.c $(LDLIBS);\
	fi

FRC :

install: all
	-if u3b2; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy512; \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy1K; \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy2K; \
	ln $(INSDIR)/dcopy1K $(INSDIR)/dcopy; \
	elif u3b15; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy1b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy2b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy; \
	else \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy1b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/dcopy; \
	fi

clean:
	rm -f *.o

clobber : clean
	-if u3b2; \
	then \
	rm -f $(TESTDIR)/dcopy512 $(TESTDIR)/dcopy1K $(TESTDIR)/dcopy2K; \
	elif u3b15; \
	then \
	rm -f $(TESTDIR)/dcopy1b $(TESTDIR)/dcopy2b $(TESTDIR)/dcopy; \
	else \
	rm -f $(TESTDIR)/dcopy1b $(TESTDIR)/dcopy; \
	fi

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fsck:fsck.mk	1.11"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc
INCRT = $(ROOT)/usr/include
INS = install
CFLAGS = -O -I$(INCRT)
LDFLAGS = -s
FRC =

all:
	-if u3b2; then make -f fsck.mk fsck512 fsck1K fsck2K; \
	elif u3b15; then make -f fsck.mk fsck1b fsck2b fsck; \
	else make -f fsck.mk fsck1b fsck; fi
	

fsck512 fsck1b::\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/fs/s5filsys.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/fs/s5inode.h\
	$(INCRT)/sys/stat.h\
	fsck.c\
	$(FRC)
	-if u3b2; \
	then \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=1 -o $(TESTDIR)/fsck512 fsck.c $(LDLIBS);\
	else \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=1 -o $(TESTDIR)/fsck1b fsck.c $(LDLIBS);\
	fi

fsck1K fsck::\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/fs/s5filsys.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/fs/s5inode.h\
	$(INCRT)/sys/stat.h\
	fsck.c\
	$(FRC)
	-if u3b2; \
	then \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=2 -o $(TESTDIR)/fsck1K fsck.c $(LDLIBS);\
	else \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=2 -o $(TESTDIR)/fsck fsck.c $(LDLIBS);\
	fi
	
	

fsck2K fsck2b::\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/fs/s5filsys.h\
	$(INCRT)/sys/fs/s5dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/fs/s5inode.h\
	$(INCRT)/sys/stat.h\
	fsck.c\
	$(FRC)
	-if u3b2; \
	then \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=4 -o $(TESTDIR)/fsck2K fsck.c $(LDLIBS);\
	elif u3b15; \
	then \
	$(CC) $(CFLAGS) $(LDFLAGS) -DFsTYPE=4 -o $(TESTDIR)/fsck2b fsck.c $(LDLIBS);\
	fi


FRC :

install: all
	-if u3b2; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck512; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck1K; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck2K; \
	ln $(INSDIR)/fsck1K $(INSDIR)/fsck; \
	elif u3b15; \
	then \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck1b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck2b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck; \
	else \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck1b; \
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck; \
	fi


clean:
	rm -f *.o

clobber : clean
	-if u3b2; \
	then \
	rm -f $(TESTDIR)/fsck512 $(TESTDIR)/fsck1K $(TESTDIR)/fsck2K; \
	elif u3b15; \
	then \
	rm -f $(TESTDIR)/fsck1b $(TESTDIR)/fsck2b $(TESTDIR)/fsck; \
	else \
	rm -f $(TESTDIR)/fsck1b $(TESTDIR)/fsck; \
	fi

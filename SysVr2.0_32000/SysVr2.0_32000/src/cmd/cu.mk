#	/*@(#)cu.mk	1.2*/
#	cu make file

CC = cc
ROOT =
INSDIR = $(ROOT)/usr/bin

all:	cu

install:	all
	cp cu $(INSDIR)
	chown root $(INSDIR)/cu
	chgrp sys $(INSDIR)/cu
	chmod 4755 $(INSDIR)/cu

cu:
	$(CC) -o cu  cu.c

clean:
	-rm -f *.o

clobber: clean
	rm -f cu

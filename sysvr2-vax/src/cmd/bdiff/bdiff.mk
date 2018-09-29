#
#	@(#)bdiff.mk	1.3
#

ROOT= 

BIN=$(ROOT)/usr/bin

CFLAGS=-c

LDFLAGS= -s -n -i

LIBS= -lPW

all:	bdiff

bdiff:	bdiff.o
	$(CC) $(LDFLAGS) -o bdiff bdiff.o $(LIBS) 

bdiff.o:	bdiff.c
	$(CC) $(CFLAGS) bdiff.c

install:	all
	rm -f $(BIN)/bdiff
	mv bdiff $(BIN)/bdiff
	chmod 755 $(BIN)/bdiff
	chgrp bin $(BIN)/bdiff
	chown bin $(BIN)/bdiff

clean:
	-rm -f bdiff.o

clobber:	clean
	-rm -f bdiff

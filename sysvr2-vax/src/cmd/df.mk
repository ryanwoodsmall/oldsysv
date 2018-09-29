#	@(#)df.mk	1.4
#	df make file
ROOT=
INCRT = $(ROOT)/usr/include
INSDIR = $(ROOT)/bin
CFLAGS = -O -s
INS=:

df:
	$(CC) -I$(INCRT) $(CFLAGS) -o df df.c
	$(INS) $(INSDIR) -m 4755 -u root -g bin df

all:	install clobber

install:
	$(MAKE) -f df.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f df

#	factor make file
#	SCCS:	@(#)factor.mk	1.1

OL = /
INS = :
INSDIR = $(OL)usr/bin
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s $(IFLAG)
SOURCE = factor.c
MAKE = make

factor:	$(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o factor factor.c -lm
	$(INS) factor $(INSDIR)

install:
	$(MAKE) -f factor.mk INS=cp OL=$(OL)

clean:
	rm -f factor.o

clobber:	clean
	  rm -f factor

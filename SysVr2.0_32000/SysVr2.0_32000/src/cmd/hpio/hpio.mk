#	@(#)hpio.mk	1.2
ROOT =
TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/usr/bin 
LDFLAGS = -s -n

all: hpio

hpio: hpio.c $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/hpio hpio.c  

install: all
	$(INS) -n $(INSDIR) $(TESTDIR)/hpio

clobber:
	-rm -f $(TESTDIR)/hpio

FRC:

# <@(#)hpio.mk	1.2>

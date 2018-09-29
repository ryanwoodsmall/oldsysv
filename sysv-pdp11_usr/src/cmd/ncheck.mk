#	@(#)ncheck.mk	1.1
TESTDIR = .
FRC =
INS = /etc/install
IFLAG = -i
LDFLAGS = -s -n
INSDIR = -n /usr/bin
CFLAGS = -O
FFLAG =
SMFLAG=

all: ncheck

ncheck: ncheck.c 
	if [ x$(IFLAG) != x-i ]  ; then \
	$(CC) $(LDFLAGS) -Dsmall=-1 $(CFLAGS)  $(IFLAG) -o $(TESTDIR)/ncheck ncheck.c ; \
	else $(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -o $(TESTDIR)/ncheck ncheck.c ; \
	fi


test:
	rtest $(TESTDIR)/ncheck

install: all
	$(INS) $(INSDIR) $(TESTDIR)/ncheck /etc

clean:

clobber: clean
	-rm -f $(TESTDIR)/ncheck

FRC:

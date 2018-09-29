#	@(#)iltd.mk	1.1
TESTDIR = .
FRC =
INS = /etc/install
INSDIR = /stand
CFLAGS = -O
FILES = iltd.o tcrt0.o prf.o console.o hthp.o tmrp.o rl.o hm.o \
	rp03boot.o rp04boot.o rm05boot.o rl11boot.o

all:	$(TESTDIR)/iltd

$(TESTDIR)/iltd: $(FILES)
	ld -s -o $(TESTDIR)/iltd $(FILES) -lc

rp03boot.o:	bthdr tail03
	cat bthdr /stand/rpboot1 tail03 >rp03boot.o

rp04boot.o:	bthdr tailrp
	cat bthdr /stand/hpboot1 tailrp >rp04boot.o

rm05boot.o:	bthdr tailrm
	cat bthdr /stand/hmboot1 tailrm >rm05boot.o

rl11boot.o:	bthdr tailrl
	cat bthdr /stand/rl11boot tailrl >rl11boot.o

$(FILES): $(FRC)

install: all
	$(INS) -f $(INSDIR) $(TESTDIR)/iltd

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/iltd

FRC:

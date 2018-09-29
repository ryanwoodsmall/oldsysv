# @(#)fscv.mk	1.1
# Description file for the Make command

LIBES =
CFLAGS = -O
TESTDIR = .
INS = /etc/install -n /etc

TYPES = /usr/include/sys/types.h
STATS = /usr/include/sys/stat.h /usr/include/ustat.h
INODES = /usr/include/sys/ino.h /usr/include/sys/inode.h
STDIO = /usr/include/stdio.h

# Check dependency of fscv on object files
all: fscv

fscv:  	ckmnt.o cnvino.o cpyblk.o fscv.o indfp.o indfv.o indino.o inop.o \
	inov.o superp.o superv.o swap.o
	$(CC) -s $(LDFLAGS) *.o -o $(TESTDIR)/fscv $(LIBES)

ckmnt.o : $(TYPES)  $(STATS)
cnvino.o : $(TYPES) $(INODES)
cpyblk.o :  $(STDIO) params.h
fscv.o :  $(STDIO) params.h super16.h super32.h
indfp.o indfv.o :  $(STDIO) indir16.h indir32.h
indino.o :  $(STDIO)
superp.o superv.o : super16.h super32.h
clean:
	rm -f *.o
clobber: clean
	rm -f $(TESTDIR)/fscv

install: all
	$(INS) $(TESTDIR)/fscv

test:
	rtest fscv

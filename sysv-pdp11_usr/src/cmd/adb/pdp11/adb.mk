#	@(#)adb.mk	1.2
TESTDIR = .
FRC =
INS = /etc/install
INSDIR =
CFLAGS = -O
IFLAG = -i
FFLAG =
OFILES = access.o command.o expr.o findfn.o format.o input.o opset.o main.o\
	 output.o pcs.o print.o runpcs.o setup.o sym.o

all: adb

adb: $(OFILES) message.o
	$(CC) $(FFLAG) $(IFLAG) -s -o $(TESTDIR)/adb *.o

$(OFILES): defs.h $(FRC)
message.o: mac.h mode.h $(FRC)

test:
	rtest $(TESTDIR)/adb

install: all
	$(INS) $(TESTDIR)/adb $(INSDIR)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/adb

FRC:

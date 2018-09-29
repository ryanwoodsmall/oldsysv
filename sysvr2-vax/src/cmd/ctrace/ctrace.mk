#	ctrace makefile
#	@(#)ctrace.mk	1.3
# installation directories:
ROOT=
BIN = $(ROOT)/usr/bin
LIB = $(ROOT)/usr/lib/ctrace

# setting preprocessor symbols:
# set for UNIX/370:
# U370 = -b1,0
CC=cc
CFLAGS = -O -DLIB=\"$(LIB) $(U370)

YFLAGS = -d

SOURCE	 = constants.h global.h main.c parser.y scanner.l lookup.c trace.c \
	   runtime.c ctcr
OTHER	 = ctrace.mk
OBJECTS	 = main.o parser.o scanner.o lookup.o trace.o

ctrace: $(OBJECTS)
	$(CC) -is $(OBJECTS) -o $@

all: ctrace

install: all
	-rm -f $(BIN)/ctc $(BIN)/ctcr $(BIN)/ctrace
	/etc/install -f $(BIN) ctrace
	/etc/install -f $(BIN) ctcr
	ln $(BIN)/ctcr $(BIN)/ctc
	-mkdir $(LIB)
	/etc/install -f $(LIB) runtime.c

clean:
	rm -f *.o y.tab.h y.output

clobber: clean
	rm -f ctrace

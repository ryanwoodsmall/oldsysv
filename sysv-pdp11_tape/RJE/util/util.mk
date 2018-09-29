#	util.mk 1.1 of 7/30/81
#	@(#)util.mk	1.1

CC = cc

LIB = ../lib/rje.a

FILES = rjestat snoop cvt

install:	all

all:	$(FILES)

rjestat:	rjestat.c atoe.h $(LIB)
	$(CC) -s -O -n -o rjestat rjestat.c $(LIB)
	cp rjestat /usr/bin/rjestat
	-chown rje /usr/bin/rjestat; chgrp bin /usr/bin/rjestat
	-chmod 4755 /usr/bin/rjestat

snoop:		snoop.c
	$(CC) -s -O -n -o snoop snoop.c
	cp snoop /usr/rje/snoop
	-chmod 755 /usr/rje/snoop
	-chown rje /usr/rje/snoop; chgrp bin /usr/rje/snoop

cvt:		cvt.c $(LIB)
	$(CC) -s -O -n -o cvt cvt.c $(LIB)
	cp cvt /usr/rje/cvt
	-chmod 755 /usr/rje/cvt
	-chown rje /usr/rje/cvt; chgrp bin /usr/rje/cvt

$(LIB):		../lib/*.c
	cd ../lib; make -f lib.mk rje.a clean

clean:
	-rm -f $(FILES)

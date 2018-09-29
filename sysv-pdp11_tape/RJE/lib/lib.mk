#	lib.mk 1.1 of 7/30/81
#	@(#)lib.mk	1.1

CC= cc
CCFLAGS= -O
FILES= reboot.o getlines.o sysname.o name.o \
	rjedead.o copy.o nmatch.o

install:	rje.a
.c.o:
	$(CC) -c $(CCFLAGS) $<

rje.a:	$(FILES)
	-rm rje.a
	ar r rje.a $(FILES)
	-chmod 644 rje.a
	-chown bin rje.a
	-chgrp bin rje.a

reboot.o:	rjedead.o

clean:
	-rm -f *.o

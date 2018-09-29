#
#	@(#)nsc.mk	1.0
#

ROOT= 
BIN=$(ROOT)/usr/nsc

CFLAGS=-O
LDFLAGS= -s
INS=:
CHG=:

PGMS= 5toB Bto5 5toG mt

all:	$(PGMS)

$(PGMS):
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c
	$(INS) $@ $(BIN)
	$(CHG) chmod 755 $(BIN)/$@
	$(CHG) chown bin $(BIN)/$@
	$(CHG) chgrp bin $(BIN)/$@

install:
	make -f nsc.mk all INS=/bin/cp CHG=

clean:
	-rm -f $(PGMS)

clobber:	clean


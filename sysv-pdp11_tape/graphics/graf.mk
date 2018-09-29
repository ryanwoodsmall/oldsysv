# @(#)graf.mk	1.1
ARGS =
ARGH =	/usr/src/arglist
BIN1 =	/usr/bin
BIN2 =	/usr/bin/graf
LIB =	/usr/lib/graf
SRC =	/usr/src/cmd/graf

install:
	-mkdir $(BIN2); chmod 755 $(BIN2)
	-mkdir $(LIB); chmod 755 $(LIB)
	-mkdir $(SRC)/lib; chmod 755 $(SRC)/lib
	if test -x $(ARGH); \
	then cd src; eval make BIN1=$(BIN1) BIN2=$(BIN2) `$(ARGH) + $(ARGS)`;\
	else cd src; make BIN1=$(BIN1) BIN2=$(BIN2); fi
	chown bin $(BIN1)/graphics; chgrp bin $(BIN1)/graphics
	chown bin `find $(BIN2) -print`; chgrp bin `find $(BIN2) -print`
	chown bin `find $(LIB) -print`; chgrp bin `find $(LIB) -print`

clobber:;

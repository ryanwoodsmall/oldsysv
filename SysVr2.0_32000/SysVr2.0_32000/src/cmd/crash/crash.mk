# @(#)crash.mk	1.1
#
# crash commands makefile
#
TARGET =
FRC =

all:
	-if vax; then cd dec; make -f crash.mk $(TARGET); fi
	-if pdp11; then cd dec; make -f crash.mk $(TARGET); fi
	-if u3b; then cd u3b20; make -f crash.mk $(TARGET); fi
	-if ns32000; then cd ns32000; make -f crash.mk $(TARGET); fi

install: 
	make -f ./crash.mk TARGET='install'
clean:
	make -f ./crash.mk TARGET='clean'
clobber: clean
	make -f ./crash.mk TARGET='clobber'

FRC:

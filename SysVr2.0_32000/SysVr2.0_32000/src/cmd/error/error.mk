# @(#)error.mk	1.3
#
# error commands makefile
#
TARGET =
FRC =

all:
	-if vax; then cd dec; make -f error.mk $(TARGET); fi
	-if pdp11; then cd dec; make -f error.mk $(TARGET); fi
	-if u3b; then cd u3b20; make -f error.mk $(TARGET); fi
	-if ns32000; then cd ns32000; make -f error.mk $(TARGET); fi

install: 
	make -f ./error.mk TARGET='install'
clean:
	make -f ./error.mk TARGET='clean'
clobber:
	make -f ./error.mk TARGET='clobber'

FRC:

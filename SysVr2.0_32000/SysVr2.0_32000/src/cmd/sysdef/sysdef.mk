# @(#)sysdef.mk	1.2
#
# sysdef commands makefile
#
TARGET =
FRC =

all:
	-if vax; then cd dec; make -f sysdef.mk $(TARGET); fi
	-if ns32000; then cd ns32000; make -f sysdef.mk $(TARGET); fi
	-if pdp11; then cd dec; make -f sysdef.mk $(TARGET); fi
	-if u3b; then cd u3b20; make -f sysdef.mk $(TARGET); fi

install: 
	make -f ./sysdef.mk TARGET='install'
clean:
	make -f ./sysdef.mk TARGET='clean'
clobber:
	make -f ./sysdef.mk TARGET='clobber'

FRC:

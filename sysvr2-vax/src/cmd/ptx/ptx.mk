#
#	@(#)ptx.mk	1.4
#

ROOT = 

ULIB = $(ROOT)/usr/lib

UBIN = $(ROOT)/usr/bin

all: ptx eign

install: all
	-cpset ptx $(UBIN)/ptx 
	-cpset eign $(ULIB)/eign 644

clean:
	-rm -f *.o

clobber: clean
	-rm  -f eign ptx

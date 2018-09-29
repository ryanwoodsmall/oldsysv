#	@(#)makefile	1.1
DIR = .

tapeboot: tapeboot.o
	strip tapeboot.o
	dd if=tapeboot.o of=tmp1 count=1 conv=sync
	cat tmp1 tmp1 > $(DIR)/tapeboot
	rm tmp1 tapeboot.o

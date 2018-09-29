# @(#)Makefile	1.1
I =	../../include
L =	/usr/lib/graf
CMDS =	dtoc ttoc vtoc
BIN =	../../bin

toc all : $(CMDS)
	touch toc

ttoc :	ttoc.sh ttoc.d
	cp ttoc.sh $(BIN)/ttoc
	chmod 755 $(BIN)/ttoc
	-mkdir $(L)/ttoc.d; chmod 755 $(L)/ttoc.d
	cd ttoc.d; ls | cpio -pl $(L)/ttoc.d
	touch ttoc

dtoc :	dtoc.sh
	cp dtoc.sh $(BIN)/dtoc
	chmod 755 $(BIN)/dtoc
	touch dtoc

vtoc :	vtoc.d $(I)
	cd vtoc.d; make BIN=$(BIN)

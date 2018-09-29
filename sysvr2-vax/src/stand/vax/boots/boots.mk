#	@(#)boots.mk	1.9
INS = :
all:	gdboot1b gdboot2b vaxboot

vaxboot::	gtboot.o tsboot.o
	ld -s ../execld.file gtboot.o
	dd if=a.out of=tmp1 bs=168 skip=1
	dd if=tmp1 of=tmp2 bs=1b conv=sync

	ld -s ../execld.file tsboot.o
	dd if=a.out of=tmp1 bs=168 skip=1
	dd if=tmp1 of=tmp3 bs=1b conv=sync
	cat tmp2 tmp3 >vaxboot
	rm a.out tmp1 tmp2 tmp3 gtboot.o tsboot.o

vaxboot::
	$(INS) -f /stand vaxboot

gtboot.o:
	as -m -w -o gtboot.o gtboot.s

tsboot.o:
	as -m -w -o tsboot.o tsboot.s

gdboot2b::  gdboot2b.o
	ld -s ../execld.file gdboot2b.o
	dd if=a.out of=tmp1 bs=168 skip=1
	dd if=tmp1 of=gdboot2b bs=1b conv=sync count=1
	rm tmp1 gdboot2b.o a.out

gdboot2b.o: gdboot2b.s gdboot2b.h
	as -m -w -o gdboot2b.o gdboot2b.s

gdboot2b::
	$(INS) -f /stand gdboot2b

gdboot1b::  gdboot1b.o
	ld -s ../execld.file gdboot1b.o
	dd if=a.out of=tmp1 bs=168 skip=1
	dd if=tmp1 of=gdboot1b bs=1b count=1
	rm tmp1 gdboot1b.o a.out

gdboot1b.o: gdboot1b.s gdboot1b.h
	as -m -w -o gdboot1b.o gdboot1b.s

gdboot1b::
	$(INS) -f /stand gdboot1b

clean:
	rm -f *.o tmp*  a.out

clobber: clean
	 rm -f tapeboot gdboot1b gdboot2b gtboot vaxboot

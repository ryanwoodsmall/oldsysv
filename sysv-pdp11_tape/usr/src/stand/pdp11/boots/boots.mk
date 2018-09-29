#	@(#)boots.mk	1.1
INS = :

all:	tapeboot diskboots boot2

tapeboot: tapeboot.o
	strip tapeboot.o
	dd if=tapeboot.o of=tmp1 count=1 conv=sync
	cat tmp1 tmp1 > tapeboot
	rm tmp1 tapeboot.o
	$(INS) -f /stand tapeboot

diskboots:
	ins=$(INS) sh -x boots.sh

boot2:
	as hp1boot.s
	strip a.out
	dd if=a.out of=hpboot1 bs=16 skip=1
	rm a.out
	as -o hpboot2 hp2boot.s
	strip hpboot2
	as rp1boot.s
	strip a.out
	dd if=a.out of=rpboot1 bs=16 skip=1
	rm a.out
	as -o rpboot2 rp2boot.s
	strip rpboot2
	as hm1boot.s
	strip a.out
	dd if=a.out of=hmboot1 bs=16 skip=1
	rm a.out
	as -o hmboot2 hm2boot.s
	strip hmboot2
	$(INS) -f /stand hpboot1
	$(INS) -f /stand hpboot2
	$(INS) -f /stand hmboot1
	$(INS) -f /stand hmboot2
	$(INS) -f /stand rpboot1
	$(INS) -f /stand rpboot2

clobber:
	rm -f tapeboot hpboot1 hpboot2 hmboot1 hmboot2 rpboot1 rpboot2
	rm -f rp03boot rp04boot rm05boot rk11boot rs04boot rl11boot

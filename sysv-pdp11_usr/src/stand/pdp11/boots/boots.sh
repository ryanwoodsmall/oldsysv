:	'@(#)boots.sh	1.1'
for i in rp03 rp04 rm05 rk11 rs04 rl11
do
	as fsboot1.s ${i}i.s fsboot2.s ${i}r.s fsboot3.s
	strip a.out
	dd if=a.out of=${i}boot bs=16 skip=1 count=32
	rm a.out
	${ins} -f /stand ${i}boot
done

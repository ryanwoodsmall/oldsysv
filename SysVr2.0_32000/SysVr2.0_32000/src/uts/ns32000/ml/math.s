	.globl	_min
	.globl	_max
_min:
	movd	4(sp),r0
	cmpd	r0,8(sp)
	bls	min1
	movd	8(sp),r0
min1:
	ret	0

_max:
	movd	4(sp),r0
	cmpd	r0,8(sp)
	bhs	max1
	movd	8(sp),r0
max1:
	ret	0

	.globl	udiv
udiv:
	movd	4(sp),r0
	movqd	0,r1
	deid	8(sp),r0
	movd	r1,r0
	ret	0

	.globl	urem
urem:
	movd	4(sp),r0
	movqd	0,r1
	deid	8(sp),r0
	ret	0

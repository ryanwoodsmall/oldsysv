	.file	"udiv.s"
.globl udiv

udiv:
	movd	4(sp),r0
	movqd	0,r1
	deid	8(sp),r0
	movd	r1,r0
	ret	0

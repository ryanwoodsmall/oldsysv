	.file	"urem.s"
.globl urem

urem:
	movd	4(sp),r0
	movqd	0,r1
	deid	8(sp),r0
	ret	0

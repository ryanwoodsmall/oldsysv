
	.file	"alloca.s"
.globl	_alloca
_alloca:
	sprd	sp,r1
	movd	r1,r0
	subd	4(sp),r0
	bicd	$3,r0
	addd	$8,r0
	lprd	sp,r0
	addd	$4,r0
	movd	0(r1),tos
	ret	0

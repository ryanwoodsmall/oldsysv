	.file	"mcount.s"
#	@(#)mcount.s	1.5
# count subroutine called during profiling

.globl	mcount
.comm	countbase,4

mcount:
	movd	0(r0),r1
	cmpqd	0,r1
	bne	.incr
	movd	countbase,r1
	cmpqd	0,r1
	beq	.return
	addd	$8,countbase
	movd	0(sp),0(r1)
	addqd	4,r1
	movd	r1,0(r0)
.incr:
	addqd	1,0(r1)
.return:
	ret	0

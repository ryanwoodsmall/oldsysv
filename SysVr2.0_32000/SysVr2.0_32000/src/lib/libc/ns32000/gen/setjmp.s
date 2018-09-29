	.file	"setjmp.s"
#	@(#)setjmp.s	1.4
# C library -- setjmp, longjmp

#	longjmp(a,v)
# will generate a "return(v)" from
# the last call to
#	setjmp(a)
# by restoring r3-r7,pc,sp,fp from 'a'
# and doing a return.
#

	.globl	_setjmp
	.globl	_longjmp
_setjmp:
	MCOUNT
	movd	0(sp),r1	#ret pc
	sprd	sp,r2		#save sp
	lprd	sp,4(sp)
	adjspb	$-32
	sprd	fp,tos
	save	[r1,r2,r3,r4,r5,r6,r7]
	lprd	sp,r2
	movqd	0,r0		#return 0
	ret	0


_longjmp:
	MCOUNT
	movd	8(sp),r0
	lprd	sp,4(sp)
	sprd	fp,r1
	cmpd	r1,28(sp)	# same frame?
	beq	same_fr
	restore	[r1,r2,r3,r4,r5,r6,r7]
	br	onw
same_fr:
	adjspb	$-20
	restore	[r1,r2]
onw:
	lprd	fp,tos
	lprd	sp,r2
	movd	r1,0(sp)
	cmpqd	0,r0
	bne	out
	movqd	1,r0
out:
	ret	0

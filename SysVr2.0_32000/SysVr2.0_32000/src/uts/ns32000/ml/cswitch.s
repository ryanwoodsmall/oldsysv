	.globl	_save
	.globl	_resume
	.globl	_setjmp
	.globl	_longjmp
_save:
_setjmp:
	sprw	psr,r0
	movd	0(sp),r1	#ret pc
	bicpsrw	$0x800
	sprd	sp,r2		#save sp
	lprd	sp,4(sp)
	adjspb	$-36
	sprd	fp,tos
	save	[r0,r1,r2,r3,r4,r5,r6,r7]
	lprd	sp,r2
	movqd	0,r0		#return 0
	bispsrw	$0x800
	ret	0

_resume:
	movd	4(sp),r2	#p->p_addr
	movd	8(sp),r4	#save area
	bicpsrw	$0x800
	ashd	$10,r2	#physical address of user lvl 1
	movd	r2,r0
	ord	$3,r0	#PG_V | PG_KW
	movd	_kpte1,r1
	movd	r0,512(r1)
	ord	$0x200,r0
	movd	r0,516(r1)
	lmr	ptb0,r1
	lmr	ptb1,r2

rout:
	lprd	sp,r4
	restore	[r0,r1,r2,r3,r4,r5,r6,r7]
	lprd	fp,tos
	lprd	sp,r2
	lshd	$16,r0
	movd	r0,0(sp)
	movd	r1,tos
	sprw	psr,r0
	movqd	1,r0
	rett	0

_longjmp:
	movd	4(sp),r4
	bicpsrw	$0x800
	br	rout

	.globl	_savfp
	.globl	_restfp
_savfp:
	movd	4(sp),r0
	sfsr	0(r0)
	movl	f0,4(r0)
	movl	f2,12(r0)
	movl	f4,20(r0)
	movl	f6,28(r0)
	ret	0
_restfp:
	movd	4(sp),r0
	lfsr	0(r0)
	movl	4(r0),f0
	movl	12(r0),f2
	movl	20(r0),f4
	movl	28(r0),f6
	ret	0

# @(#)random.s	6.1
	.text
	.align 4
Xrandom:
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random
	.align 4;	bsbw	random

	.globl	_stray
	.align	4
random:
	pushr	$0x3f
	movl	24(sp),r0
	subl2	$Xrandom+3,r0
	pushl	r0
	calls	$1,_stray
	popr	$0x3f
	tstl	(sp)+
	rei

# @(#)userio.s	6.2
	.align	4
_fubyte:	.globl	_fubyte
	.word	0x0000
	prober	$3,$1,*4(ap)	# byte accessible ?
	beql	eret			# no
	movzbl	*4(ap),r0
	ret

	.align	4
_subyte:	.globl	_subyte
	.word	0x0000
	probew	$3,$1,*4(ap)	# byte accessible ?
	bneq	ok_su
	movl	4(ap), r0
	jsb	mptea
	jeql	eret
	bbc	$24, (r0), eret	# no copy on write bit set
ok_su:
	movb	8(ap),*4(ap)
	clrl	r0
	ret

	.align	4
_fuword:	.globl	_fuword
_fuiword:.globl	_fuiword
	.word	0x0000
	prober	$3,$4,*4(ap)
	beql	eret
	movl	*4(ap),r0
	ret

	.align	4
_suword:	.globl	_suword
_suiword:.globl	_suiword
	.word	0x0000
	probew	$3,$4,*4(ap)
	bneq	ok_sui
	movl	4(ap), r0
	jsb	mptea
	jeql	eret
	bbc	$24, (r0), eret	# no copy on write bit set
	movl	4(ap), r0
	addl2	$3, r0
	jsb	mptea
	jeql	eret
	bbc	$24, (r0), eret	# no copy on write bit set
ok_sui:
	movl	8(ap),*4(ap)
	clrl	r0
	ret
eret:
	mnegl	$1,r0			# error return
	ret


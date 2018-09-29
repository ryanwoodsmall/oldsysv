#	@(#)misc.s	1.1
	.align	2
_spl1:	.globl	_spl1
	.word	0x0000
	mfpr	$IPL,r0		# get IPL value
	mtpr	$2,$IPL		# disable RESCHED & AST interrupts
	ret

	.align	2
_spl4:	.globl	_spl4
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	$0x14,$IPL	# disable bus level 4 interrupts
	ret

	.align	2
_spl5:	.globl	_spl5
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	$0x15,$IPL	# disable bus level 5 interrupts
	ret

	.align	2
_spl6:	.globl	_spl6
_spl7:	.globl	_spl7
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	$0x18,$IPL	# disable bus level 7 and clock interrupts
	ret

	.align	2
_spl0:	.globl	_spl0
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	$0,$IPL
	ret

	.align	2
_splx:	.globl	_splx
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	4(ap),$IPL
	ret

	.align	2
_getipl: .globl	_getipl
	.word	0x0000
	mfpr	$IPL,r0
	ret

# @(#)copy.s	6.2.1.1
	.align	4
	.globl	_copyseg
	.globl	_copypage
_copyseg:
_copypage:
	.word	0x0000
	mfpr	$IPL,-(sp)
	mtpr	$HIGH,$IPL	# turn off interrupts
	movl	_copypte,r0
	bisl3	$PG_V|PG_KR,4(ap),(r0)+
	bisl3	$PG_V|PG_KW,8(ap),(r0)
	movl	_copyvad,r0
	addl3	r0,$512,r1
	mtpr	r0,$TBIS	# invalidate entry for copy 
	mtpr	r1,$TBIS
	movc3	$512,(r0),(r1)
	mtpr	(sp)+,$IPL
	ret

	.align	4
	.globl	_clearseg
	.globl	_clearpage
_clearseg:
_clearpage:
	.word	0x0000
	mfpr	$IPL,-(sp)
	mtpr	$HIGH,$IPL	# extreme pri level
	movl	_copypte,r0
	bisl3	$PG_V|PG_KW,4(ap),(r0)
	movl	_copyvad,r0
	mtpr	r0,$TBIS
	movc5	$0,(r0),$0,$512,(r0)
	mtpr	(sp)+,$IPL
	ret

	.align	4
	.globl	_copyin
_copyin:
	.word	0x0000
	movl	12(ap),r0
	movl	4(ap),r1
	cmpl	$512,r0
	bgeq	cishort
ciloop:
	prober	$3,$512,(r1)
	beql	ceret
	addl2	$512,r1
	acbl	$513,$-512,r0,ciloop
cishort:
	prober	$3,r0,(r1)
	beql	ceret
	brb	copymov

	.set	cw, 0x1000000		# cw bit in pte
	.align	4
	.globl	_copyout
_copyout:
	.word	0x0000
	movl	12(ap),r0
	movl	8(ap),r1
	cmpl	$512,r0
	bgeq	coshort
coloop:
	probew	$3,$1,(r1)
	beql	ccw1
ccw2:
	addl2	$512,r1
	acbl	$513,$-512,r0,coloop
coshort:
	probew	$3,$1,(r1)
	beql	ccw3
ccw4:
	addl2	r0, r1
	subl2	$1, r1
	probew	$3, $1, (r1)
	beql	ccw5
copymov:
	movc3	12(ap),*4(ap),*8(ap)
	clrl	r0
	ret
ceret:
	mnegl	$1,r0
	ret
ccw1:	# if this page is marked copy on write, then we'll ignore
	# the probew failure
	pushr	$0x3		# these register are going to be bashed
	movl	r1, r0		# right here
	jsb	mptea		# get the pte address
	jeql	ccwerr		# no pte
	bitl	$cw, (r0)	# check cw bit in pte
	jeql	ccwerr		# not cw; probew was right
	popr	$0x3
	jbr	ccw2
ccwerr:
	popr	$0x3
	jbr	ceret
ccw3:
	pushr	$0x3		# these register are going to be bashed
	movl	r1, r0		# right here
	jsb	mptea		# get the pte address
	jeql	ccwerr		# no pte
	bitl	$cw, (r0)	# check cw bit in pte
	jeql	ccwerr		# not cw; probew was right
	popr	$0x3
	jbr	ccw4
ccw5:
	pushr	$0x3		# these register are going to be bashed
	movl	r1, r0		# right here
	jsb	mptea		# get the pte address
	jeql	ccwerr		# no pte
	bitl	$cw, (r0)	# check cw bit in pte
	jeql	ccwerr		# not cw; probew was right
	popr	$0x3
	jbr	copymov

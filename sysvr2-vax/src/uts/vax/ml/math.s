# @(#)math.s	6.1
# unsigned int divide:
#	i = udiv((int)dvdnd, (int)divis)
#
# unsigned int remainder:
#	j = urem((int)dvdnd, (int)divis)
#
# also include C library like interface, main line case only!
	.text
	.align	4
	.globl	_udiv
	.globl	udiv
	.globl	_urem
	.globl	urem
_udiv:
udiv:
	.word	0000
	movl	4(ap),r0	# dividend
	clrl	r1
	ediv	8(ap),r0,r0,r1	# quotient in r0
	ret

	.align 	4
_urem:
urem:
	.word	0000
	movl	4(ap),r0
	clrl	r1
	ediv	8(ap),r0,r1,r0	# remainder in r0
	ret

	.align	4
	.globl	_min
	.globl	_max
_min:
	.word	0000
	movl	4(ap),r0
	cmpl	r0,8(ap)
	blssu	min1
	movl	8(ap),r0
min1:
	ret

	.align	4
_max:
	.word	0000
	movl	4(ap),r0
	cmpl	r0,8(ap)
	bgequ	max1
	movl	8(ap),r0
max1:
	ret

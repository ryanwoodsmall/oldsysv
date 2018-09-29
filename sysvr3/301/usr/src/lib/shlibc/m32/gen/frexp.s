#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"frexp.s"
.ident	"@(#)libc-m32:gen/frexp.s	1.5"
#------------------------------------------------------------------------------
#	double frexp (value, eptr)
#		double value;
#		int *eptr;
#
#	Frexp breaks "value" up into a fraction and an exponent.
#	It stores the exponent indirectly through eptr, and
#	returns the fraction.  More specifically, after
#
#		double d, frexp();
#		int e;
#		d = frexp (x, &e);
#
#	then |d| will be less than 1, and x will be equal to d*(2**e).
#	Further, if x is not zero, d will be no less than 1/2, and if
#	x is zero, both d and e will be zero too.

# NOTE: For infinity and NAN, it returns the same number, and sets
#	e to 0x7ff, which is the reserved exponent.
#	IT DOES NOT TRAP ON Trapping NANs.
#------------------------------------------------------------------------------
	.data
	.set	DEXP_BIAS,1023		# bias for double precision exponent
	.set	MAX_EXP,0x7ff		# reserved exponent for nan or infinity
	.set	DEXP_OFFSET,20		# bit offset where the exponent starts
	.set	DEXP_WIDTH,11		# width of double exponent
	.set	SIGN_MASK,0x7fffffff	# used to mask off the sign
	.set	SIGN_BIT,0x80000000	# to check sign bit
	.set	IMPLIED_BIT,0x100000	# to check implied bit
#------------------------------------------------------------------------------
	.text
	.align	4
	.def    frexp; .val	frexp;	.scl    2;	.type	047;	.endef
	.globl	frexp
#------------------------------------------------------------------------------
frexp:
	MCOUNT
	movw	0(%ap),%r0		# r0 = sign+exponent+high part of mant
	movw	4(%ap),%r1		# r1 = low part of mantissa
	andw3	&SIGN_MASK,%r0,%r2	# bits other than sign
	orw2	%r1,%r2			# OR of all the bits other than sign
	jz	.ret_zero		# If zero, we're done
	EXTFW	&DEXP_WIDTH-1,&DEXP_OFFSET,%r0,%r2	# get exponent
	jz	.denormal		# If exponent zero, value is denormalized
	cmpw	%r2,&MAX_EXP
	je	.nan_inf
#------------------------------------------------------------------------------
.norm:
	insv	&DEXP_BIAS-1,&DEXP_OFFSET,&DEXP_WIDTH,%r0# Force the result
					# exponent to biased 0
	subw3	&DEXP_BIAS-1,%r2,*8(%ap)# Bias the exponent appropriately
	RET
#------------------------------------------------------------------------------
.ret_zero:				# return zero simply returns the input
					# so that sign is preserved.
	movw	%r2,*8(%ap)
	RET
#------------------------------------------------------------------------------
.denormal:				# number is denormal
	addw2	&1,%r2			# because exponent bias
					# for denormal numbers is -126
					# while normal bias is -127
	pushw	%r8			# we need nore storage
	pushw	%r7
	andw3	&SIGN_BIT,%r0,%r7	# remember sign

.norm_loop:				# normalization loop
	lrsw3	&31,%r1,%r8		# get MSB of low part
	llsw2	&1,%r0			# high word << 1
	orw2	%r8,%r0			# high word | msb of low word
	llsw2	&1,%r1			# low word << 1
	subw2	&1,%r2			# exp--
	bitw	&IMPLIED_BIT,%r0	# is fraction normalized?
	je	.norm_loop		# 	no, right shift again
					# yes,  %r2 has the right exponent
	orw2	%r7,%r0			# put the sign back
	POPW	%r7
	POPW	%r8			# restore registers
	jmp	.norm			# and join main code
#--------------------------------------------------------------------------
.nan_inf:				# if the input operand is
					# is nan or infinity, simply
					# return it, and return exponent=7ff
					# could have shared .ret_zero code
					# but will be diffrent in future.
	movw	&MAX_EXP,*8(%ap)
	RET

	.def	frexp;	.val	.;	.scl	-1;	.endef

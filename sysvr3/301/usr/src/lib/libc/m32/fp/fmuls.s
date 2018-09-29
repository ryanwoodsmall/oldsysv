#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fmuls.s"
.ident	"@(#)libc-m32:fp/fmuls.s	1.5"

#	float _fmuls(srcF1,srcF2)
#	float srcF1,srcF2;
#
	.data
	.set	FP_MULT,0x2		# type of operation = MULT
	.set	FORMAT,0x03030303	# format string for rounding
					# exceptions

	.set	sign_bit,0x80000000	# to check the sign
	.set	sign_mask,0x7fffffff	# to check bits other than sign
	.set	exp_bias,0x7f		# bias for exponent
	.set	exp_offset,23		# for extracting the exponents
	.set	exp_width,8		#	" "
	.set	mant_width,23		# for extracting the mantissa
	.set	implied_1,0x800000	# to set/check the implied bit
	.set	overflow_bit,0x1000000

	.set	max_exp,0xff		# the reserved value for NaN and Inf
	.set	infinity,0x7f800000	# single precision infinity

	.set	lo_bits,0xfff

	.set	sticky,0x1
	.set	GRS,0x7

	.globl	_fp_hw			# to check hardware presence
	.globl	_asr			# the simulated arithmetic status


#	Register usage:
#	%r5	argument2 exponent
#	%r6	arguemnt1 exponent (and result exponent)
#	%r7	argument2 mantissa
#	%r8	argument1 mantissa (and result mantissa)
#	%r4	switch
#	%r3	normalize
#	%r2	shift needed
#	%r0,%r1,%r2 are scratch

##############################################################################
	.text
	.align	4
	.def	_fmuls;	.val	_fmuls;	.scl	2;	.type	046;	.endef
	.globl	_fmuls
_fmuls:
	TSTW	_fp_hw
	je	.software
#-----------------------------------------
# MAU is present in the system.
	mfmuls2	4(%ap),0(%ap)		# Execute mau instructions.
	movw	0(%ap),%r0
	RET
#-----------------------------------------
	.align	4
.software:
	save	&6

	movw	0(%ap),%r3			# get arg1
	extzv	&exp_offset,&exp_width,%r3,%r5	# %r5 = exp1
	cmpw	%r5,&max_exp			# if (exp1 == ff)
	je	.nan_infinity			#	arg1 is nan/infinity

	movw	4(%ap),%r4			# get arg2
	extzv	&exp_offset,&exp_width,%r4,%r6	# %r6 = exp2
	cmpw	%r6,&max_exp			# if (exp2 == ff)
	je	.nan_infinity			#	arg2 is nan/infinity

	extzv	&0,&mant_width,%r3,%r3		# %r3 = mant1
	TSTW	%r5				# if (exp1 == 0)
	je	.zero_denormal1			#	arg2 is zero/denormal
	orw2	&implied_1,%r3			# else	put in implied one
.normalized1:
	extzv	&0,&mant_width,%r4,%r4		# %r4 = mant2
	TSTW	%r6				# if (exp2 == 0)
	je	.zero_denormal2			#	arg2 is zero/denormal
	orw2	&implied_1,%r4			# else	put in implied one
.normalized2:

	addw2	%r5,%r6				# exp1 + exp2
	subw2	&exp_bias,%r6			# exp = exp1 + exp2 - bias

#
#	we do four 12 bit multiplies
#
	andw3	&lo_bits,%r3,%r0		# l1
	andw3	&lo_bits,%r4,%r1		# l2

	mulw3	%r0,%r1,%r2			# rl = l1 * l2

	lrsw2	&12,%r3				# h1
	lrsw2	&12,%r4				# h2

	mulw3	%r3,%r4,%r8			# h1 * h2

	mulw3	%r0,%r4,%r7			# l1 * h2
	mulw3	%r1,%r3,%r0			# l2 * h1
	addw2	%r0,%r7				# l1*h2 + l2*h1
	lrsw3	&12,%r2,%r0			# high(l1*l2)
	addw2	%r0,%r7				# rm = l1*h2 + l2*h1 + high(rl)

	lrsw3	&12,%r7,%r0
	addw2	%r0,%r8				# rh = h1*h2 + high(rm)
						# %r8 has most significant 24 bits

	andw2	&lo_bits,%r7			# %r7 has next 12 bits
	andw2	&lo_bits,%r2			# %r2 has next 12 bits
	bitw	&implied_1,%r8
	je	.shift_needed
	addw2	&1,%r6
	jmp	.sticky
.shift_needed:
	llsw2	&1,%r8
	lrsw3	&11,%r7,%r0
	orw2	%r0,%r8
	llsw2	&1,%r7
	andw2	&lo_bits,%r7			# ???
.sticky:
	TSTW	%r2
	je	.no_low_bits
	orw2	&sticky,%r7
.no_low_bits:
	andw3	&0x7ff,%r7,%r0			# take lowest 11 bits
	lrsw2	&10,%r7				# sticky >> 10 to get R
	TSTW	%r0				# if (lowest 11 != 0)
	je	.sign
	orw2	&sticky,%r7			#	put sticky
	andw2	&3,%r7				# to mask off any other bits
#----------------------------------------------------------------------------
#	now format as expected in the rounding code
	.globl	_round.single
#	%r0	aligned sign
#	%r6	exponent
#	%r8	mantissa
#	%r7	RS bits
#	%r5	format string
#	%r3	type of operation
.round:
.sign:
	xorw3	0(%ap),4(%ap),%r0
	andw2	&sign_bit,%r0			# sign = sign1 xor sign2
	movw	&FORMAT,%r5
	movw	&FP_MULT,%r3
	jmp	_round.single
#-----------------------------------------------------------------------------
.zero_denormal1:
	TSTW	%r3			# if (arg1 == 0.0)
	je	.signed_zero		#	return zero
					# else	normalize the number
	addw2	&1,%r5			# denormal numbers have bias 126

.norm_loop1:
	subw2	&1,%r5
	llsw2	&1,%r3
	bitw	&implied_1,%r3
	je	.norm_loop1
	jmp	.normalized1		#	join main line code
#-----------------------------------------------------------------------------
# we have to return a zero with right sign.
.signed_zero:
	xorw3	0(%ap),4(%ap),%r0	# calculate sign
	andw2	&sign_bit,%r0		# just the sign bit is needed
	jmp	.ret6
#-----------------------------------------------------------------------------
.zero_denormal2:			# argument 2 has exponent == 0
	TSTW	%r4			# if (arg2 == 0.0)
	je	.signed_zero		#	return zero
					# else	adjust for denormal number
	addw2	&1,%r6			# denormal numbers have bias 126

.norm_loop2:
	subw2	&1,%r6
	llsw2	&1,%r4
	bitw	&implied_1,%r4
	je	.norm_loop2
	jmp	.normalized2		#	join main line code
#-----------------------------------------------------------------------------
# special cases. The shared code is in the file "dspecial.s"
#
	.data
.special_table:
	.word	_invalid.single		# arg1 is a trapping NaN
	.word	_invalid.single		# arg2 is a trapping NaN
	.word	_qnan1.single		# arg1 is a quiet NaN
	.word	_qnan2.single		# arg2 is a quiet NaN
	.word	.inf12			# both arguments are infinities
	.word	.inf1			# arg1 is infinity
	.word	.inf2			# arg2 is infinity
#-----------------------------------------------------------------------------
	.text
.nan_infinity:
	JSB	_type.single		# find the type of the operands
	llsw2	&2,%r0			# index *= 4
	movw	&FP_MULT,%r1
	jmp	*.special_table(%r0)	# jump to appropiate code
#----------------------------------------
.inf1:
	andw3	&sign_mask,4(%ap),%r0	# if (argment2 == zero)
	jz	_invalid.single		#	then invalid operation
					# else return infinity of correct sign
#---------------------------------------
.inf12:					# both arguments are infinity
	xorw3	0(%ap),4(%ap),%r0
	andw2	&sign_bit,%r0
	orw2	&infinity,%r0
.ret6:
	ret	&6
#---------------------------------------
.inf2:
	andw3	&sign_mask,0(%ap),%r0	# if (argment1 != zero)
	jnz	.inf12			# 	infinity of correct sign
	jmp	_invalid.single		# else invalid operation
#-----------------------------------------------------------------------
	.def	_fmuls;	.val	.;	.scl	-1;	.endef


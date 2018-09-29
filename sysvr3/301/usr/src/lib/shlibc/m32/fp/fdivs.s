#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fdivs.s"
.ident	"@(#)libc-m32:fp/fdivs.s	1.6"

#	float _fdivs(srcF1,srcF2)
#	float srcF1,srcF2;
#	Division of single precision IEEE format numbers

	.data
	.set	FP_DIV,0x3		# type of operation = DIV
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

	.set	chunk,13
	.set	lo_bits,0x1fff

	.set	sticky,0x1
	.set	GRS,0x7

	.set	count,0
	.set	local_var,4

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
	.def	_fdivs;	.val	_fdivs;	.scl	2;	.type	046;	.endef
	.globl	_fdivs
_fdivs:
	TSTW	_fp_hw
	je	.software
#-----------------------------------------
# MAU is present in the system.
	mfdivs2	4(%ap),0(%ap)		# Execute mau instructions.
	movw	0(%ap),%r0
	RET
#-----------------------------------------
	.align	4
.software:
	save	&6
	addw2	&local_var,%sp

	movw	0(%ap),%r3			# get dividend
	extzv	&exp_offset,&exp_width,%r3,%r4	# %r4 = exp1
	cmpw	%r4,&max_exp			# if (exp1 == ff)
	je	.nan_infinity			#	dividend is nan/infinity

	movw	4(%ap),%r5			# get divisor
	extzv	&exp_offset,&exp_width,%r5,%r6	# %r6 = exp2
	cmpw	%r6,&max_exp			# if (exp2 == ff)
	je	.nan_infinity			#	divisor is nan/infinity
#----------------------------------------------------------------------------
	extzv	&0,&mant_width,%r3,%r3		# %r3 = dividend mantissa

	TSTW	%r4				# if (exp1 == 0)
	je	.zero_denormal1			#	dividend is zero/denormal
	orw2	&implied_1,%r3			# else	put in implied one
.normalized1:
#----------------------------------------------------------------------------
	extzv	&0,&mant_width,%r5,%r5		# %r4 = divisor mantissa

	TSTW	%r6				# if (exp2 == 0)
	je	.zero_denormal2			#	divisor is zero/denormal
	orw2	&implied_1,%r5			# else	put in implied one
.normalized2:
#----------------------------------------------------------------------------
	subw3	%r6,%r4,%r6			# exp1 - exp2
	addw2	&exp_bias,%r6			# exp = exp1 - exp2 + bias
#----------------------------------------------------------------------------
#	Division is done by the algorithm described in Knuth.
#	Since both the dividend and divisor are aligned, the
#	first bit of result is either 0 or 1, and we make
#	use of the fact.
#	Two more quotient chunks, 13 bits each are generated;
#	so that we have 26 or 27 bits of significant quotient.
#	( we need at least 25 bits of significant quotient)
#
#	%r3 is dividend
#	%r4 is dividend extension
#	%r5 is divisor
#	%r6 is exponent
#	%r7 is trail quotient q^
#	%r8 is partail quotient
#	%r1,%r2 is the partial product = v(1)v(2) * q^
#----------------------------------------------------------------------------
	movw	&2,count(%fp)	# no of times in loop
	movw	&0,%r4		# clear dividend extension

	llsw2	&2,%r3
	llsw2	&2,%r5

				# First bit can be only zero or one.
	movw	&0,%r8		# partial quotient = 0
	cmpw	%r3,%r5
	jl	.div_loop
	movw	&1,%r8
	subw2	%r5,%r3
#----------------------------------------------------------------------------
.div_loop:
	llsw2	&chunk,%r8		# partail quotient << 13
					# (place for next chunk)

	extzv	&chunk,&chunk,%r3,%r0	# u(j)
	extzv	&chunk,&chunk,%r5,%r1	# v(1)
	cmpw	%r0,%r1			# if u(j) == v(1)
	jne	.q_hat
	movw	&0x1fff,%r7		#	q^ = 1fff
	jmp	.mul_sub
.q_hat:
	udivw3	%r1,%r3,%r7		# q^ = u(j)u(j+1) / v(1)
	jz	.loop_end
#----------------------------------------------------------------------------
.mul_sub:
	extzv	&chunk,&chunk,%r5,%r1	# v(1)
	mulw2	%r7,%r1			# v(1)* q^
	andw3	&lo_bits,%r5,%r2	# v(2)
	mulw2	%r7,%r2			# v(2)* q^

	extzv	&chunk,&chunk,%r2,%r0	# carry from v(2) * q^
	addw2	%r0,%r1			# is added to v(1) * q^
	llsw2	&32-chunk,%r2		# and masked from v(2) * q^

	subw2	%r2,%r4			# subtract low bits
	BCCB	.no_borrow
	subw2	&1,%r3			# subtract carry
.no_borrow:
	subw2	%r1,%r3			# subtract high bits
	jmp	.check
#---------------------------------
.add_back:
	llsw3	&32-chunk,%r5,%r0
	addw2	%r0,%r4
	BCCB	.no_carry
	addw2	&1,%r3
.no_carry:
#??	addw2	%r5,%r3
	lrsw3	&chunk,%r5,%r0
	addw2	%r0,%r3
	subw2	&1,%r7			# q^ --

	.set	BORROW,0x02000		# BORROW = 1 << chunk
.check:					# did we subtract too much?
	bitw	&BORROW,%r3
	jne	.add_back
#---------------------------------
.loop_end:
	orw2	%r7,%r8			# insert quotient chunk in quotient

	extzv	&32-chunk,&chunk,%r4,%r0# align the dividend
	llsw2	&chunk,%r3
	orw2	%r0,%r3
	llsw2	&chunk,%r4

	subw2	&1,count(%fp)
	jnz	.div_loop
#----------------------------------------------------------------------------
#	now format as expected in the rounding code
#	%r0	aligned sign
#	%r3	type of operation
#	%r5	format string for exception
#	%r6	exponent
#	%r8	mantissa
#	%r7	RS bits
.round:
	bitw	&0x4000000,%r8		# check units bit (after 26 bits)
	jnz	.units			# if (!zero) we have units place
					# else	we have fraction quotient
	subw2	&1,%r6			#	exponent--
	llsw2	&1,%r8			#	mant << 1
.units:
	andw3	&GRS,%r8,%r7
	lrsw2	&3,%r8

	andw3	&sticky,%r7,%r0
	lrsw2	&1,%r7
	orw2	%r0,%r7

	orw2	%r3,%r4
	je	.sign
	orw2	&sticky,%r7
.sign:
	xorw3	0(%ap),4(%ap),%r0
	andw2	&sign_bit,%r0		# sign = sign1 xor sign2
	.globl	_round.single
	movw	&FORMAT,%r5		# format string for rounding exceptions
	movw	&FP_DIV,%r3		# type of operation
	jmp	_round.single
#-----------------------------------------------------------------------------
.zero_denormal1:
	TSTW	%r3			# if (dividend != 0.0)
	jne	.norm1			#	normalize it
	andw3	&sign_mask,%r5,%r0	#	else	if (divisor !=0)
	jne	.signed_zero		#			return zero
					#		else	invalid
	movw	&FP_DIV,%r1
	jmp	_invalid.single
#-----------------------------------------------------------------------------
.norm1:					# normalize the dividend
	addw2	&1,%r4			# denormal numbers have bias 126
.norm_loop:				# do
	subw2	&1,%r4			#	exp--
	llsw2	&1,%r3			#	mantissa << 1
	bitw	&implied_1,%r3		# while (mantissa&implied_bit == 0)
	je	.norm_loop
	jmp	.normalized1		#	join main line code
#-----------------------------------------------------------------------------
# we have to return a zero with right sign.
.signed_zero:
	xorw3	0(%ap),4(%ap),%r0	# calculate sign
	andw2	&sign_bit,%r0		# just the sign bit is needed
	jmp	.ret6
#-----------------------------------------------------------------------------
.zero_denormal2:			# argument 2 has exponent == 0
	TSTW	%r5			# if (arg2 == 0.0)
	je	.div_zero		#	divide by zero exception
					# else	normalize the number
	addw2	&1,%r6			# note:denormal numbers have bias 126
.norm_loop2:				# do
	subw2	&1,%r6			#	exp2--
	llsw2	&1,%r5			#	mantissa2 << 1
	bitw	&implied_1,%r5		# while (mantissa2&implied_bit == 0)
	je	.norm_loop2
	jmp	.normalized2		# join main line code
#-----------------------------------------------------------------------------
	.data
	.set	FP_DIVZ,6		# divide by zero exception
	.set	DIVZ_STICKY,0x40	# to set divide by zero sticky
	.set	DIVZ_MASK,0x800		# to check the mask
#-----------------------------------------------------------------------------
	.text
.div_zero:				# divide by zero exception
	orw2	&DIVZ_STICKY,_asr	# set divide by zero sticky
	bitw	&DIVZ_MASK,_asr		# if (mask disabled)
	je	.inf1			# result is correctly signed infinity
					# else	raise trap
	movw	&FP_DIV,%r1		#	type of operation
	movw	&FP_DIVZ,%r3		#	type of fault
	jmp	_trap.single
#-----------------------------------------------------------------------------
# special cases. The shared code is in the file "sspecial.s"
#
	.data
.special_table:
	.word	_invalid.single		# arg1 is a trapping NaN
	.word	_invalid.single		# arg2 is a trapping NaN
	.word	_qnan1.single		# arg1 is a quiet NaN
	.word	_qnan2.single		# arg2 is a quiet NaN
	.word	_invalid.single		# both arguments are infinities
	.word	.inf1			# arg1 is infinity
	.word	.signed_zero		# arg2 is infinity
#-----------------------
	.text
.nan_infinity:
	JSB	_type.single		# find the type of the operands
	movw	&FP_DIV,%r1		# %r1 = type of operation
	llsw2	&2,%r0			# index *= 4
	jmp	*.special_table(%r0)	# jump to appropiate code
#----------------------------------------
.inf1:
	xorw3	0(%ap),4(%ap),%r0	# return infinity of correct sign
	andw2	&sign_bit,%r0
	orw2	&infinity,%r0
.ret6:
	ret	&6
#-----------------------------------------------------------------------
	.def	_fdivs;	.val	.;	.scl	-1;	.endef


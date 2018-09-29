#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fdivd.s"
.ident	"@(#)libc-m32:fp/fdivd.s	1.6"

#	Division of two IEEE double precision arguments.
#	double	_fdivd(dividend,divisor)
#	double	dividend,divisor;
#

# REGISTER USAGE:

#	%r3	v(1)v(2)
#	%r4	inner loop count(k)
#	%r5	trial quotient q(j)
#	%r6	loop count(j)
#	%r7	pointer to divisor (v)
# 	%r8	pointer to dividnend (u)

	.data
	.set	FP_DIV,3		# type of operation.

	.set	EXP_OFFSET,20		# offset for extracting exponent
	.set	EXP_WIDTH,11		# width of exponent
	.set	BIAS,1023		# exponent bias for double precision
	.set	MAX_EXP,0x7ff		# the reserved value for NaN and Inf
	.set	MASK,0xfffff		# to mask out the sign and exponent
	.set	IMPLIED_BIT,0x100000	# or in this value to get implied bit
	.set	SIGN_BIT,0x80000000	# to check/set just the sign bit
	.set	INFINITY,0x7ff00000	# high word of double infinity

	.set	u0,0			# place for u(0) (1 word)
	.set	u,u0+4			# unpacked dividend (4 words)
	.set	v,u+16			# unpacked divisor (2 words)
	.set	q,v+8			# stack storage for quotient (3 words)
	.set	result_exp,q+12		# storage for result exponent (1 word)
	.set	trap_result,result_exp+4# return value from exception (2 words)

	.set	local_var,trap_result+8	# total size of local variables

##############################################################################
	.text
	.align	4
	.def	_fdivd;	.val	_fdivd;	.scl	2;	.type	047;	.endef
	.globl	_fdivd
#----------------------------------------------------------------------------
_fdivd:
	MCOUNT
	TSTW	_fp_hw			# is hardware present
	je	.software		# no hardware
#-----------------------------------------------------------------------------
	mfdivd2	8(%ap),0(%ap)		# MAU present
	movw	4(%ap),%r1		# return the result in %r0,%r1
	movw	0(%ap),%r0
	RET
#-----------------------------------------------------------------------------
.nan_inf1:		# the only use of these jumps is
.nan_inf2:		# branch optimization in main line code.
	jmp	.nan_inf
#------------------------
.zero_denorm1:
	jmp	.z_d_1
#------------------------
.zero_denorm2:
	jmp	.z_d_2
#-----------------------------------------------------------------------------
	.align	4
.software:
	save	&6			# save registers
	addw2	local_var,%sp		# adjust stack for locals
#---------Is the dividend NaN or Infinity? ----------------------
	movw	0(%ap),%r7		# get dividend high word
	extzv	&EXP_OFFSET,&EXP_WIDTH,%r7,%r5	# dividned exponent
	cmpw	%r5,&MAX_EXP		# if (dividend exp == 7ff)
	je	.nan_inf1		#	dividend is infinity or NaN
#---------Is the divisor NaN or Infinity? ------------------------------------
	movw	8(%ap),%r3		# get divisor high word
	extzv	&EXP_OFFSET,&EXP_WIDTH,%r3,%r6	# divisor exponent
	cmpw	%r6,&MAX_EXP		# if (divisor exponent == 7ff)
	je	.nan_inf2		#	divisor is infinity or NaN
#-----------------------------------------------------------------------------
	movw	4(%ap),%r8		# get dividend low word
	andw2	&MASK,%r7 		# mask off exponent and sign of dividend
	movw	12(%ap),%r4		# get divisor low part
	andw2	&MASK,%r3 		# mask off exponent and sign of divisor
#---------Is the dividend zero/denormal? Normalize it -------------------------

	TSTW	%r5
	je	.zero_denorm1		# dividend is zero or denormal

	orw2	&IMPLIED_BIT,%r7	# insert the implied 1
.denorm1:				# reentry point for denormal dividend

					# normalize by shifting left 11 bits
	movw	&0,u0(%fp)		# u(0) = 0
	llsw2	&11,%r7
	extzv	&21,&11,%r8,%r0		# get 11 bits from low order
	orw3	%r0,%r7,u(%fp)		# put u(1)u(2)
	llsw3	&11,%r8,u+4(%fp)	# put u(3)u(4)

	movw	&0,u+8(%fp)		# clear u(5)u(6)
	movw	&0,u+12(%fp)		# clear u(7)u(8)

#---------Is the divisor zero/denormal?. Normalize it --------------------
	TSTW	%r6
	je	.zero_denorm2		# divisor is zero or denormalized

	orw2	&IMPLIED_BIT,%r3	# insert the implied 1
.denorm2:				# reentry point for denormal divisor

	llsw2	&11,%r3
	extzv	&21,&11,%r4,%r0		# get 11 bits from low order
	orw2	%r0,%r3			# %r3 = v(1)v(2)
	llsw2	&11,%r4			# %r4 = v(3)v(4)
	movaw	v(%fp),%r7		# %r7 = & v(1)
	movw	%r3,0(%r7)		# write v(1)v(2)
	movw	%r4,4(%r7)		# write v(3)v(4)

#-------- division of two numbers ----------------------------------------

	subw2	%r6,%r5			# exp = divided_exp - divisor_exp
	addw3	&BIAS,%r5,result_exp(%fp)# bias and store
					# the result exponent
#-------------------------------------------------------------------------
	movw	&-2,%r6			# (loop count - 1 ) *  2
					# used to index half words

# since the first quotient can be only 0 or 1. We assume 1
	movaw	u0+2(%fp),%r8		# &u(0)
	movw	&1,%r5			# trial q(0)
	SUBW2	{uword}%r4,u+4(%fp)	# u(3,4) -= v(3,4)
	jgeu	.sub2
	subw2	&1,u(%fp)		# u(1,2) -= borrow
.sub2:
	SUBW2	{uword}%r3,u(%fp)	# u(1,2) -= v(1,2)
	jgeu	.loop_check
	subh2	&1,u0+2(%fp)		# u(0) -= borrow
.sub3:
	jmp	.loop_check		# now to check if 1 is too big
#---------The divison loop for the rest -------------------------------
.div_loop:
	movaw	u(%fp),%r8		# %r8 = &u(1)
	addw2	%r6,%r8			# %r8 = &u(1) + (j-1) = &u(j)

	lrsw3	&16,%r3,%r2		# v(1)
	MOVH	{uhalf}(%r8),%r5	# u(j)
	cmpw	%r5,%r2			# if ( u(j) == v(1) )
	jne	.q_hat
	movw	&0xffff,%r5		#	q^ = 0xffff (b-1)
	jmp	.mul_sub

.q_hat:
	llsw2	&16,%r5			# u(j-1) * b
	MOVH	{uhalf}2(%r8),%r0	# u(j+1)
	orw2	%r0,%r5			# u(j)*b + u(j+1)

	udivw2	%r2,%r5			# q^ = u(j)*b + u(j+1) / v(1)
	jz	.loop_end		# if q^ == 0, skip

#-------------- Calculate u(j,j+3) - v*q^ ------------------------
.mul_sub:
	movw	&6,%r4			# inner loop count(k)
	movw	&0,%r2			# carry = 0
.ms_loop:
	addw3	%r4,%r7,%r0		# &v(0) + k = &v(k)
	MOVH	{uhalf}0(%r0),%r0	# v(k)
	mulw2	%r5,%r0			# v(k) * q^
	addw2	%r2,%r0			# v(k) * q^ + borrow
	addw3	%r4,%r8,%r1		# & u(j+k)
	addw2	&2,%r1			# & u(j+k+1)
	SUBH2	{uhalf}%r0,0(%r1)	# compute u(j+k) -v(k) * q^ - borrow
	BCCB	.ms1
	addw2	&0x10000,%r0		# increment borrow
.ms1:
	lrsw3	&16,%r0,%r2		# borrow
	subw2	&2,%r4			# k = k-1
	jnneg	.ms_loop
	SUBH2	{uhalf}%r2,-2(%r1)	# u(j) -= borrow
#--------------------------------------------------------------------
.loop_check:
	BCCB	.loop_end		# if no borrow was generated
					# 	this iteration is done
.add_back:
	subw2	&1,%r5			# q(j) --
	movw	&6,%r4			# k
	movw	&0,%r2			# carry = 0
.add_loop:
	addw3	%r4,%r7,%r0		# &v(k)
	MOVH	{uhalf}0(%r0),%r0	# v(k)
	addw2	%r2,%r0			# v(k) + carry
	addw3	%r4,%r8,%r1		# & u(j+k)
	addw2	&2,%r1			# & u(j+k+1)
	ADDH2	{uhalf}%r0,0(%r1)	# u(j+k) + v(k) + carry
	BCCB	.al2
	addw2	&0x10000,%r0		# increment carry
.al2:
	lrsw3	&16,%r0,%r2		# carry from v(k) + u(j+k)+ carry
	subw2	&2,%r4			# k = k - 1
	jnneg	.add_loop
	ADDH2	{uhalf}%r2,-2(%r1)	# u(j+k-1)  -= carry

	BCCB	.add_back
#-------------------------------------------------------------------------
.loop_end:
	movaw	q(%fp),%r0		# &q
	addw2	%r6,%r0			# &q + (j-1)
	movh	%r5,2(%r0)		# save q(j)

	addw2	&2,%r6			#  loop counter(j) += 2;
	cmpw	%r6,&6
	jle	.div_loop
#--------- Now put back in the format needed by rounding code ------------
	movw	q(%fp),%r6		# get the quotient(high)
	movw	q+4(%fp),%r7		# 	  quotient(low)
	movw	q+8(%fp),%r8		#	  q(5)q(6)
	bitw	&0x10000,%r6		# get the units bit
					# if (!zero) we have units place
					# else	we have a fractional q
	jnz	.units1
	subw2	&1,result_exp(%fp)	# exponent --
	llsw2	&5,%r6			# shift by 5 to normalize
	lrsw3	&27,%r7,%r0		# 5 bits form low word
	orw2	%r0,%r6			# is put in high word
	llsw2	&5,%r7			# low word << 5
	lrsw3	&27,%r8,%r0		# 5 bits form sticky word
	orw2	%r0,%r7			# is put in low word
	llsw3	&5,%r8,%r0		# put R and sticky bits in %r0
	lrsw3	&31,%r0,%r8		# get R bit
	llsw2	&1,%r8			# shift it in place
	bitw	&0x7fe00000,%r0		# check the 10 sticky bits
					# (calculated 16 bits - R bit - 5 bits)
	je	.no_sticky
	jmp	.sticky
.units1:
	llsw2	&4,%r6			# shift by 4 to normalize
	lrsw3	&28,%r7,%r0		# 4 bits from low word
	orw2	%r0,%r6			# is put into hight word
	llsw2	&4,%r7			# low word << 4
	lrsw3	&28,%r8,%r0		# 4 bits from sticky part
	orw2	%r0,%r7			# is put into low word
	llsw3	&4,%r8,%r0		# put R and sticky bits in %r0
	lrsw3	&31,%r0,%r8		# get R bit
	llsw2	&1,%r8			# shift it in place
	bitw	&0x7ff00000,%r0		# check the 11 sticky bits
					# (calculated 16 bits - R bit - 4 bits)
	je	.no_sticky		# no sticky calculated in quotient
.sticky:
	orw2	&1,%r8			# set sticky
.no_sticky:
	orw3	u+8(%fp),u+12(%fp),%r0	# if (dividend became zero)
	jz	.exact			#	no sticky from there
	orw2	&1,%r8			# else	set sticky
.exact:
	movw	&FP_DIV,%r3		# %r3 = type of operation
	xorw3	0(%ap),8(%ap),%r4	# sign is xor of operand signs
	lrsw2	&31,%r4			# %r4 = the sign bit
	movw	result_exp(%fp),%r5	# %r5 = exponent
	.globl	_round.double
	jmp	_round.double
#--------------------------------------------------------------------------
# special cases
	.globl	_type.double
	.globl	_invalid.double
	.globl	_qnan1.double
	.globl	_qnan2.double
	.data
	.set	TNAN1,0		# arg1 is a trapping NaN
	.set	TNAN2,1		# arg2 is a trapping NaN
	.set	QNAN1,2		# arg1 is a quiet NaN
	.set	QNAN2,3		# arg2 is a quiet NaN
	.set	INF12,4		# both arguments are infinities
	.set	INF1,5		# arg1 is infinity
	.set	INF2,6		# arg2 is infinity
.special_table:
	.word	_invalid.double
	.word	_invalid.double
	.word	_qnan1.double
	.word	_qnan2.double
	.word	_invalid.double
	.word	.inf1
	.word	.signed_zero
#-----------------------
	.text
.nan_inf:
	JSB	_type.double		# find the type of the operands
	llsw2	&2,%r0			# index *= 4
	movw	&FP_DIV,%r1
	jmp	*.special_table(%r0)	# jump to appropiate code
#----------------------------------------------------------------------------
.inf1:					# return infinity of correct sign
	xorw3	0(%ap),8(%ap),%r0
	andw2	&SIGN_BIT,%r0
	orw2	&INFINITY,%r0
	movw	&0,%r1
	ret	&6
#-----------------------------------------------------------------------------
.z_d_1:
	orw3	%r7,%r8,%r0		# if (arg1 != 0.0)
	je	.zero1			#	adjust for denormal number
	jmp	.norm12
.norm1:
	subw2	&1,%r5			# exp--
.norm12:				# normalize the number, adjust exponent
	llsw2	&1,%r7			# high word <<1
	lrsw3	&31,%r8,%r0		# highest bit of low word
	orw2	%r0,%r7			# is put in high word
	llsw2	&1,%r8			# low word <<1
	bitw	&IMPLIED_BIT,%r7	# if ( !nornalized )
	je	.norm1			#	repeat
	jmp	.denorm1		# else join main line code
#-----------------------------------------------------------------------------
.zero1:
	orw3	%r3,%r4,%r0
	orw2	%r6,%r0			# if arg2 is also zero
	jne	.signed_zero		# then	return signed zero
.invalid:				# else	invalid operation
	movw	&FP_DIV,%r1
	jmp	_invalid.double
#-----------------------------------------------------------------------------
# we have to return a zero with right sign.
.signed_zero:
	xorw3	0(%ap),8(%ap),%r0	# calculate sign
	andw2	&SIGN_BIT,%r0		# just the sign bit is needed
	movw	&0,%r1			# zero the low word
	ret	&6
#-----------------------------------------------------------------------------
.z_d_2:
	orw3	%r3,%r4,%r0		# if (arg2 == 0.0)
	je	.div_zero		#	return divide by zero exception
					# else	adjust for denormal number
	jmp	.norm22
.norm2:
	subw2	&1,%r6			# exp--
.norm22:				# normalize the number, adjust exponent
	llsw2	&1,%r3			# high word <<1
	lrsw3	&31,%r4,%r0		# highest bit of low word
	orw2	%r0,%r3			# is put in high word
	llsw2	&1,%r4			# low word <<1
	bitw	&IMPLIED_BIT,%r3	# if ( !nornalized )
	je	.norm2			#	repeat
	jmp	.denorm2		# else join main line code
#-----------------------------------------------------------------------------
	.data
	.set	FP_DIVZ,6		# divide by zero exception
	.set	FP_D,4			# type of operands
	.set	FP_NULL,0
	.set	FP_UNION,0xc		# size of return value
	.set	DIVZ_STICKY,0x40	# to set divide by zero sticky
	.set	DIVZ_MASK,0x800		# to check the mask
.format:
	.byte	FP_D,FP_D,FP_NULL,FP_D
	.text
.div_zero:				# divide by zero exception
	orw2	&DIVZ_STICKY,_asr	# set divide by zero sticky
	bitw	&DIVZ_MASK,_asr		# the the mask
	je	.inf1			# if no trap occurs, the result
					# is correctly signed infinity
	movaw	0(%sp),%r2		# pointer to return value
	addw2	&FP_UNION,%sp		# space for return value
	pushw	&FP_DIVZ		# type of exception
	pushw	&FP_DIV			# type of operation
	pushaw	.format			# format of operand etc.
	pushaw	0(%ap)			# address of operand 1
	pushaw	8(%ap)			# address of operand 2
	pushw	&FP_NULL		# no rounded result
	call	&6,_getfltsw		# raise exception
	movw	4(%r0),%r1		# get the return vaule
	movw	0(%r0),%r0
	ret	&6
#-----------------------------------------------------------------------------
	.def	_fdivd;	.val	.;	.scl	-1;	.endef


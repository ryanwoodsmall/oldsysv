#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fmuld.s"
.ident	"@(#)libc-m32:fp/fmuld.s	1.8"

#	Multiplication of two IEEE double precision arguments.
#	double _fmuld(src1,src2)
#	double src1,src2;

# REGISTER USAGE:

#	%r4	carry
#	%r5	loop counter
#	%r6	pointer to intermediate result
# 	%r7	pointer to unpacked operand 1
#	%r8	pointer to unpacked operand 2

	.data
	.set	FP_MULT,2		# type of operation.(as in <p74.h>)
	.set	EXP_OFFSET,20		# offset for extracting exponent
	.set	EXP_WIDTH,11		# width of exponent
	.set	BIAS,1023		# exponent bias for double precision
	.set	MAX_EXP,0x7ff		# the reserved value for NaN and Inf
	.set	SIGN_MASK,0x7fffffff	# to mask off sign
	.set	MASK,0xfffff		# to mask out the sign and exponent
	.set	IMPLIED_BIT,0x100000	# or in this value to get implied bit
	.set	OVERFLOW_BIT,0x200000
	.set	SIGN_BIT,0x80000000
	.set	INFINITY,0x7ff00000	# high word for infinity

# The following variables are allocated on stack
	.set	result,0		# stack storage for result (8 words)
	.set	src1,result+32		# unpacked src1 (4 words)
	.set	src2,src1+16		# unpacked src2 (4 words)
	.set	result_exp,src2+16	# storage for result exponent (1 word)

	.set	local_var,result_exp+4	# total size of local variables

##############################################################################
	.text
	.align	4
	.def	_fmuld;	.val	_fmuld;	.scl	2;	.type	047;	.endef
	.globl	_fmuld
_fmuld:
	MCOUNT
	TSTW	_fp_hw			# if (hardware present)
	je	.software		#	no, emulate
#-----------------------------------------------------------------------------
# MAU instruction
	mfmuld2	8(%ap),0(%ap)
	movw	4(%ap),%r1
	movw	0(%ap),%r0
	RET
#-----------------------------------------------------------------------------
.nan_inf1:				# These extra label optimizes branches
.nan_inf2:				# in the main line code
	jmp	.nan_inf
#-----------------------------------------------------------------------------
.zero_denorm1:				# argument 1 has exponent == 0
	orw3	%r3,4(%ap),%r0		# if (arg1 != 0.0)
	je	.signed_zero		#	return zero
					# else	adjust for denormal number
					# i.e. that the number is
	jmp	.norm_loop12		# shifted left one place
.norm_loop1:				# denormal numbers have bias 126
	subw2	&1,result_exp(%fp)
.norm_loop12:
	lrsw3	&31,%r2,%r0
	llsw2	&1,%r3
	orw2	%r0,%r3
	llsw2	&1,%r2
	bitw	&IMPLIED_BIT,%r3
	je	.norm_loop1
	jmp	.denorm1		#	join main line code
#-----------------------------------------------------------------------------
# we have to return a zero with right sign.
.signed_zero:
	xorw3	0(%ap),8(%ap),%r0	# calculate sign
	andw2	&SIGN_BIT,%r0		# just the sign bit is needed
	movw	&0,%r1			# zero the low word
	ret	&6
#-----------------------------------------------------------------------------
.zero_denorm2:				# argument 2 has exponent == 0
	orw3	%r4,12(%ap),%r0		# if (arg2 == 0.0)
	je	.signed_zero		#	return zero
	jmp	.norm_loop22		# else	adjust for denormal number
.norm_loop2:				# denormal numbers have bias 126
	subw2	&1,result_exp(%fp)
.norm_loop22:
	lrsw3	&31,%r2,%r0
	llsw2	&1,%r4
	orw2	%r0,%r4
	llsw2	&1,%r2
	bitw	&IMPLIED_BIT,%r4
	je	.norm_loop2
	jmp	.denorm2		#	join main line code

#--------- ENTRY POINT OF SOFTWARE EMULATION --------------------------------
	.align	4
.software:
	save	&6
	addw2	local_var,%sp

	movw	0(%ap),%r3		# get argument 1 high word
	extzv	&EXP_OFFSET,&EXP_WIDTH,%r3,%r0	# operand 1 exponent
	cmpw	%r0,&MAX_EXP
	je	.nan_inf1		# argument 1 is infinity or NaN

	movw	8(%ap),%r4		# get argument 2 high word
	extzv	&EXP_OFFSET,&EXP_WIDTH,%r4,%r1	# operand 2 exponent
	cmpw	%r1,&MAX_EXP
	je	.nan_inf2		# argument 2 is infinity or NaN
#----------------------- calculate result exponent ------------------------
	addw3	%r1,%r0,%r5
	subw2	&BIAS,%r5		# result exponent
	movw	%r5,result_exp(%fp)	# store the result exponent
#----------------------- unpack operand 1 ----------------------------------
	movaw	src1(%fp),%r7		# pointer to operand 1
	andw2	&MASK,%r3 		# mask off exponent and sign of op1
	movw	4(%ap),%r2		# get low part of op 1
	TSTW	%r0
	je	.zero_denorm1		# arg1 is zero or denormal

	orw3	%r2,%r3,%r0		# if (arg1 mantissa == 0)
	je	.skip1			# check if calculation can be skipped
.go_back1:				# no, it didn't work

	orw2	&IMPLIED_BIT,%r3	# insert the implied 1
.denorm1:				# reentry point for denormal arg 1
	lrsw3	&16,%r3,0(%r7)		# put in high chunk of op 1
	andw3	&0xffff,%r3,4(%r7)

	lrsw3	&16,%r2,8(%r7)		# put the low chunk of op1
	andw3	&0xffff,%r2,12(%r7)

#----------------------- unpack operand 2 ----------------------------------
	movaw	src2(%fp),%r8		# pointer to operand 2
	movw	12(%ap),%r2		# get low part of op 2
	andw2	&MASK,%r4 		# mask off exponent and sign of op2

	TSTW	%r1
	je	.zero_denorm2		# arg2 is zero or denormalized

	orw3	%r2,%r4,%r0		# if (arg2 mantissa ==0)
	je	.skip2			# check if calculation can be skipped
.go_back2:				# no, it didn't work

	orw2	&IMPLIED_BIT,%r4	# insert the implied 1
.denorm2:				# reentry point for denormal arg 2
	lrsw3	&16,%r4,0(%r8)
	andw3	&0xffff,%r4,4(%r8)

	lrsw3	&16,%r2,8(%r8)
	andw3	&0xffff,%r2,12(%r8)	# put the low chunk of op 2

#-------------------- multiplication ----------------------------------------
	movaw	result(%fp),%r6		# pointer to result
	movw	&0,16(%r6)		# clear the
	movw	&0,20(%r6)		# low part of
	movw	&0,24(%r6)		# the result
	movw	&0,28(%r6)		# (These are accessed before write)

	movw	&12,%r5			# (loop count - 1 ) *  4
					# used to index words
.mul_loop:
	addw3	%r5,%r6,%r3		# pointer to right place in result
	addw3	%r5,%r8,%r2		# pointer to right chunk in op 2
	movw	0(%r2),%r2		# get that chunk
	andw2	&0xffff,%r2		# musk off bits over 16
	jnz	.non_zero		# if it is zero
	movw	&0,0(%r3)		#	just clear the top
	jmp	.loopend		#	and skip the body of loop
.non_zero:
	mulw3	12(%r7),%r2,%r0
	addw2	16(%r3),%r0		# this halfword of result += product
	andw3	&0xffff,%r0,16(%r3)
	lrsw3	&16,%r0,%r4		# save carry

	mulw3	8(%r7),%r2,%r0
	addw2	12(%r3),%r4
	addw2	%r4,%r0
	andw3	&0xffff,%r0,12(%r3)
	lrsw3	&16,%r0,%r4

	mulw3	4(%r7),%r2,%r0
	addw2	8(%r3),%r4
	addw2	%r4,%r0
	andw3	&0xffff,%r0,8(%r3)
	lrsw3	&16,%r0,%r4

	mulw3	0(%r7),%r2,%r0
	addw2	4(%r3),%r4
	addw2	%r4,%r0
	andw3	&0xffff,%r0,4(%r3)

	lrsw3	&16,%r0,0(%r3)		# write out the most significant
					# part of the product

.loopend:
	subw2	&4,%r5			#  decrement loop counter
	jnneg	.mul_loop
#---------------------------------------------------------------------
# The result is the low 106 bits of the 128 bit result var.
# Discard the top 22 bits	bits 0-21
# Align the next 53 bits	bits 22-77
# Compress the rest in RS bits	bits 77-127

	movw	result+4(%fp),%r6
	llsw2	&16,%r6
	orw2	result+8(%fp),%r6

	movw	result+12(%fp),%r7
	llsw2	&16,%r7
	orw2	result+16(%fp),%r7
# we have bits 17-81 of the result

	extzv	&3,&1,%r7,%r8			# get the round bit
	llsw2	&1,%r8				# R <<= 1

# get the rest of the bits to calculate sticky
	orw3	result+20(%fp),result+24(%fp),%r0
	orw2	result+28(%fp),%r0
	andw3	&0x7,%r7,%r1			# lowest 3 bits of the result
	orw2	%r1,%r0
	je	.no_sticky
	orw2	&1,%r8
.no_sticky:

	lrsw2	&4,%r7			# align the low bits
	andw3	&0xf,%r6,%r0		# get 4 bits from high order
	llsw2	&28,%r0			# align them
	orw2	%r0,%r7			# put in low order bits
	lrsw2	&4,%r6			# align the high bits

	movw	result_exp(%fp),%r5
	bitw	&OVERFLOW_BIT,%r6	# if (overflow)
	je	.no_overflow
					# right shift one bit
	andw3	&1,%r8,%r0
	lrsw2	&1,%r8			# sticky >>= 1
	orw2	%r0,%r8
	andw3	&1,%r7,%r0		# get new R bit
	llsw2	&1,%r0			# align R
	orw2	%r0,%r8			# R | sticky

	lrsw2	&1,%r7			# low order bits >>= 1
	andw3	&1,%r6,%r0		# lowest bit of high order
	llsw2	&31,%r0			# shift in place
	orw2	%r0,%r7			# and put in the low order bits
	lrsw2	&1,%r6			# high order bits >>= 1
	addw2	&1,%r5			# exponent ++

.no_overflow:
	movw	&FP_MULT,%r3		# type of operation
	xorw3	0(%ap),8(%ap),%r4	# calculate sign
	lrsw2	&31,%r4			# we just want the sign bit
	.globl	_round.double		# in file "dround.s"
	jmp	_round.double		# jump to rounding code
#------------------------------------------------------------------------------
.skip1:			# argument 1 has mantissa == 0
	TSTW	%r5			# if (result exp <= 0)
	jle	.go_back1
	cmpw	%r5,&MAX_EXP		# if (result exp >= MAX_EXP)
	jge	.go_back1
	TSTW	%r1			# if (arg2 exp == 0)
	je	.go_back1
	movw	8(%ap),%r0		# result mantissa high = arg2 high
	movw	12(%ap),%r1		# result mantissa low = arg2 low
	insv	%r5,&20,&11,%r0		# result exponent
	andw3	&SIGN_BIT,0(%ap),%r5	# arg1 sign
	xorw2	%r5,%r0			# result sign
	ret	&6
#-----------------------------------------------------------------------------
.skip2:			# argument 2 has mantissa == 0
	TSTW	%r5			# if (result exp <= 0)
	jle	.go_back2
	cmpw	%r5,&MAX_EXP		# if (result exp >= MAX_EXP)
	jge	.go_back2
	TSTW	%r0			# if (arg1 exp == 0)
	je	.go_back2
	movw	0(%ap),%r0		# result mantissa high = arg1 high
	movw	4(%ap),%r1		# result mantissa low = arg1 low
	insv	%r5,&20,&11,%r0		# result exponent
	andw3	&SIGN_BIT,8(%ap),%r5	# arg2 sign
	xorw2	%r5,%r0			# result sign
	ret	&6
#------------------------------------------------------------------------------
# special cases. The code is in file "dspecial.s"
	.globl	_type.double
	.globl	_invalid.double
	.globl	_qnan1.double
	.globl	_qnan2.double

	.data
.special_table:
	.word	_invalid.double		# arg1 is a trapping NaN
	.word	_invalid.double		# arg2 is a trapping NaN
	.word	_qnan1.double		# arg1 is a quiet NaN
	.word	_qnan2.double		# arg2 is a quiet NaN
	.word	.inf12			# both arguments are infinities
	.word	.inf1			# arg1 is infinity
	.word	.inf2			# arg2 is infinity
#-----------------------
	.text
.nan_inf:
	JSB	_type.double		# find the type of the operands
	llsw2	&2,%r0			# index *= 4
	movw	&FP_MULT,%r1
	jmp	*.special_table(%r0)	# jump to appropiate code
#----------------------------------------
.inf1:
	andw3	&SIGN_MASK,8(%ap),%r0	# bits other than sign
	orw2	12(%ap),%r0		# if (argment2 == zero)
	jz	_invalid.double		#	then invalid operation
					# else return infinity of correct sign
#---------------------------------------
.inf12:					# both arguments are infinity
	xorw3	0(%ap),8(%ap),%r0	# return infinity of correct sign
	andw2	&SIGN_BIT,%r0
	orw2	&INFINITY,%r0
	movw	&0,%r1
	ret	&6
#---------------------------------------
.inf2:
	andw3	&SIGN_MASK,0(%ap),%r0	# bits other than sign
	orw2	4(%ap),%r0		# if (argment1 != zero)
	jnz	.inf12			# 	infinity of correct sign
	jmp	_invalid.double		# else invalid operation
#-----------------------------------------------------------------------------
	.def	_fmuld;	.val	.;	.scl	-1;	.endef


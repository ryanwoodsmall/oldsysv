#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"faddd.s"
.ident	"@(#)libc-m32:fp/faddd.s	1.10.1.1"
# 	double _faddd(arg1,arg2)
#	double arg1,arg2;
#	/* Addition of two double precision IEEE numbers */

	.data
	.set	FP_ADD,0		# operation types as defined
	.set	FP_SUB,1		# in ieeefp.h

	.set	BIAS,1023		# exponent bias for double precision
	.set	EXP_OFFSET,20		# offset for extracting exponent
	.set	EXP_WIDTH,11		# width of exponent
	.set	MAX_EXP,0x7ff		# the reserved value for NaN and Inf
	.set	MASK,0xfffff		# to mask off sign and exponent
	.set	IMPLIED_BIT,0x100000	# used to OR in the implied bit
	.set	SIGN_BIT,0x80000000	# to check if sign bit is on
#
#	The mantissas are unpacked as shown:
#
#	  31  30  29				    0	bit positions
#	+---+---+------------------------------------+
#	| V | I |   	bits 51 to 22		     |	high word of mantissa
#	+---+---+------------------------------------+
#	  ^   ^
#	  |   |
#	  |   +----------------------------------------	implicit 1 bit
#	  +--------------------------------------------	place for overflow
#
	.set	I_SET,0x40000000	# to check if either operand is normal
#
#	 31		 10   9   8   7	 6	   0	bit positions
#	+---------------+---+---+---+---+-----------+
#	|  bits 21 to 1	| L | G | R | S |  unused   |	low word of mantissa
#	+---------------+---+---+---+---+-----------+
#			  ^   ^   ^   ^
#			  |   |   |   |
#			  |   |   |   +---------------- sticky
#			  |   |   +-------------------- round bit
#			  |   +------------------------	guard bit
#			  +----------------------------	least significant bit

	.set	STICKY,0x80		# mask for stikcy bit
#-----------------------------------------------------------------------------
# REGISTER USAGE ( in actual computation ):
#	%r5	= result exponent
#	%r7,%r8 = result mantissa <high,low>
#	%r7,%r8 = operand 2 mantissa
#
#	%r3,%r4	= operand 1 mantissa
#
#	%r0,%r1,%r2,%r6 are scratch

# These are variables allocated on stack, and referenced through
# the frame pointer.
	.set	switch,0		# word to hold the flag "switch"
					# to remember if operands are switched
	.set	op_type,switch+4	# to remember what operation ADD/SUB
	.set	normal,op_type+4	# flag to remember if either operand
					# is normalized.

	.set	local_vars,normal+4	# total size of local variables
#-----------------------------------------------------------------------------
	.globl	_asr
	.globl	_fp_hw
##############################################################################
	.text
	.align	4
	.def	_faddd;	.val	_faddd;	.scl	2;	.type	047;	.endef
	.globl	_faddd
#-----------------------------------------------------------------------------
_faddd:
	MCOUNT
	TSTW	_fp_hw			# test hardware presence
	je	.software		# no hardware is present
#-----------------------------------------------------------------------------
# MAU code
	mfaddd2	8(%ap),0(%ap)		# execute MAU opcode
	movw	4(%ap),%r1		# return the result in %r0,%r1
	movw	0(%ap),%r0
	RET
#-----------------------------------------------------------------------------
.nan_inf1:			# the only function of these
.nan_inf2:			# labels is to optimize the branches
	jmp	.nan_inf	# in the main line code
.zero_denorm1:
	jmp	.z_d_1
.zero_denorm2:
	jmp	.z_d_2
#-----------------------------------------------------------------------------
	.align	4
.software:
	save	&6
	addw2	&local_vars,%sp		# place for temporaries
	movw	&FP_ADD,op_type(%fp)	# rememeber type of operation

.sub_entry:
	movw	0(%ap),%r3		# get argument 1 high word
	extzv	&EXP_OFFSET,&EXP_WIDTH,%r3,%r5	# get argument 1 exponent
	cmpw	%r5,&MAX_EXP		# if (exponent 1 = 7ff)
	je	.nan_inf1		# argument 1 is infinity or NaN

	movw	8(%ap),%r7		# get argument 2 high word
	extzv	&EXP_OFFSET,&EXP_WIDTH,%r7,%r2	# get argument 2 exponent
	cmpw	%r2,&MAX_EXP		# if (exponent 2 = 7ff)
	je	.nan_inf2		# argument 2 is infinity or NaN

	andw2	&MASK,%r3		# frac1 high &= MASK
	TSTW	%r5			# if (exponent 1 == 0)
	je	.zero_denorm1		# arg1 is zero or denormalized

	orw2	&IMPLIED_BIT,%r3	# frac1 high |= IMPLIED_BIT
	llsw2	&10,%r3			# frac1 high <<= 10
	movw	4(%ap),%r4		# get frac1 low
	lrsw3	&22,%r4,%r0		# get 10 high bits out of frac1 low
	orw2	%r0,%r3			# frac1 high |= low bits
	llsw2	&10,%r4			# frac1 low <<= 10
.denorm1:				# reenter if arg 1 is denormalized

	andw2	&MASK,%r7		# frac2 high &= MASK
	TSTW	%r2			# if (exponent 2 == 0)
	je	.zero_denorm2		# arg2 is zero or denormalized

	orw2	&IMPLIED_BIT,%r7	# frac2 high |= IMPLIED_BIT
	llsw2	&10,%r7			# frac2 high <<= 10
	movw	12(%ap),%r8		# get frac2 low
	lrsw3	&22,%r8,%r0		# get 10 high bits out of frac2 low
	orw2	%r0,%r7			# frac2 high |= low bits
	llsw2	&10,%r8			# frac2 low <<= 10
.denorm2:				# reenter if arg2 is denormalized
#----------------------------------------------------------------------------
	movw	&1,normal(%fp)		# set normalize flag
	orw3	%r3,%r7,%r0		# frac high1 | frac high2
	bitw	&I_SET,%r0		# if (either is normalized)
	jne	.res_exp		#	skip
	movw	&0,normal(%fp)		# else	clear normalize flag

	.align	4
.res_exp:
	CLRW	switch(%fp)
	subw3	%r2,%r5,%r6		# shift = exp1 - exp2
					# if (exp2 > exp1)
	jneg	.switch			# 	switch the operands;

	TSTW	%r6			# if ( shift == 0 )
	je	.align_end		#	bypass the alignment
	jmp	.check_alignment	# 
#------------------------------------------------------------------------------
.switch:
	movw	&0xffffffff,switch(%fp)	# switch flag = !switch flag
	mnegw	%r6,%r6			# shift = -shift
	movw	%r2,%r5			# exp = exp2

	movw	%r3,%r0
	movw	%r7,%r3			# swap frac1 high and frac2 high
	movw	%r0,%r7

	movw	%r4,%r0
	movw	%r8,%r4			# swap frac1 low and frac2 low
	movw	%r0,%r8
#-----------------------------------------------------------------------------
.check_alignment:			
	cmpw	%r6,&55			# if (shift !> maximum shift)
	jl	.align1			#	we have to align operand 2
					# else 
					# operand 2 is zero by denormalization
	orw3	%r7,%r8,%r0		# is there any 1 bit in op2?
	movw	%r3,%r7			# result mant high = op1 mant high
	movw	%r4,%r8			# result mant low  = op1 mant low
	TSTW	%r0			# if ( bits lost != 0 )
	jne	.sticky_op		#	we still have to subtract
					# else	bypass operation
	xorw3	0(%ap),8(%ap),%r0	# if (sign1 != sign2)
	jneg	.sub_sign		# 	bypass subtract
	jmp	.add_sign		# else	bypass addition
.sticky_op:
	orw2	&STICKY,%r8		# else	OR in sticky
	xorw3	0(%ap),8(%ap),%r0	# if (sign1 != sign2)
	jge	.add_sign		#	bypass addition
					# else subtract sticky
	movw	&0,%r7			#	clear the high word op2
	movw	&STICKY,%r8		#	just sticky in low word op2
	jmp	.sub_magnitude		#	and subtract
#------------------------------------------------------------------------------
	.align	4
.align1:
	movw	&24,%r2			# shiftcount = 24
					# can do maximum 24 bit shift at a time
.align2:
	cmpw	%r6,%r2			# if (shift > 24)
	jge	.align3			# then shiftcount = 24
	movw	%r6,%r2			# else shiftcount = shift
.align3:
#	WATCH OUT!
#	The IS25  instruction:	extzv	offset,width,src,dest becomes
#	32000  instruction:	EXTFW	width+1,offset,src,dest
#	syntax for		INSFW	width+1,offest,src,dest

	subw3	&1,%r2,%r1		# %r1 = shiftcount - 1
	EXTFW	%r1,&7,%r8,%r0		# get the bits to be shifted out
	lrsw2	%r2,%r8			# low order >>= shiftcount
	andw2	&0xffffff80,%r8		# gets rid of junk in low seven bits
	TSTW	%r0			# if ( bits lost != 0 )
	je	.align4			#
	orw2	&STICKY,%r8		# 	set sticky
.align4:
	EXTFW	%r1,&0,%r7,%r0		# shiftcount bits of high order
	lrsw2	%r2,%r7			# high order >>= shiftcount
	subw3	%r2,&32,%r2		# offset for insert = 32 - shiftcount
	INSFW	%r1,%r2,%r0,%r8		# is inserted into low order
	subw3	%r2,&32,%r2		# restore the value of %r2

	subw2	%r2,%r6			# total shift -= shiftcount
	jpos	.align2			# if ( shift > 0 ) loop
.align_end:
	xorw3	0(%ap),8(%ap),%r0	# if (sign1 != sign2)
	jneg	.sub_magnitude		# 	we need to subtract
#--------------------------- magnitde addition ------------------------------
.add_magnitude:
	ADDW2	{uword}%r4,%r8		# add the low order bits
	BVCB	.add_upper
	INCW	%r7			# add in the carry

.add_upper:
	addw2	%r3,%r7			# add the high order bits
	TSTW	%r7
	jge	.add_sign
					# we have one bit overflow
	andw3	&STICKY,%r8,%r0		# get the bit to be lost(sticky)
	lrsw2	&1,%r8			# low order bits >>= 1
	orw2	%r0,%r8			# low order bits |= sticky
	andw3	&1,%r7,%r0		# take the bit to be shifted out
	llsw2	&31,%r0			# align it in place
	orw2	%r0,%r8			# and shift in onto low order bits
	lrsw2	&1,%r7			# high order bits >>= 1
	INCW	%r5			# exponent++
.add_sign:
	CLRW	%r4			# sign = 0
	TSTW	0(%ap)			# if operand 1 is negative
	jge	.round
	MCOMW	%r4,%r4			# sign = !sign
#------------------------- end of magnitde addition -----------------------
	.globl	_round.double
# The inputs to rounding code are 
#	type of operation 	  = %r3 ( as defined in <ieeefp.h> )
#	sign flag		  = %r4
#	exponent		  = %r5
#	the high part of fraction = %r6
#	the low part of fraction  = %r7
#	RS bits			  = %r8

.round:
	lrsw3	&10,%r7,%r6		# high 21 bits of fraction
	llsw2	&22,%r7			# next 10 bits of fraction
	lrsw3	&10,%r8,%r0		# lowest 22 bits of fraction
	orw2	%r0,%r7
					# round only takes R and S
					# so we make S = R|S, and R = G
	extzv	&7,&3,%r8,%r8		# GRS bits
	andw3	&1,%r8,%r0		# take the S bit
	lrsw2	&1,%r8			# GRS >> 1
	orw2	%r0,%r8			# S = S | R (squash the R and S bits)
	movw	op_type(%fp),%r3
	jne	.sign_restore		# if (operation==subtract)
	jmp	_round.double
.sign_restore:
	xorw2	&SIGN_BIT,8(%ap)	#	invert the sign of operand 2
	jmp	_round.double
#--------------------------------------------------------------------------
	.align	4
.sub_magnitude:				# magnitude subtraction
	cmpw	%r3,%r7			# if ( op 1 < op2 )
	jl	.sub_rev		#	do the reverse subtract
	jg	.sub
	CMPW	{uword}%r8,%r4		# same test for low order bits
	jlu	.sub_rev
.sub:
	SUBW3	{uword}%r8,%r4,%r8	# subtract the low order bits
	jgeu	.sub_upper
	DECW	%r3			# subtract borrow
.sub_upper:
	SUBW3	%r7,%r3,%r7		# subtract high order bits
	jmp	.sub_norm
#-------------------------------------------------
					# we have a negative result
					# do the reverse subtract
.sub_rev:
	MCOMW	switch(%fp),switch(%fp)
	SUBW2	{uword}%r4,%r8
	BGEUB	.reverse_upper
	DECW	%r7			# subtract borrow
.reverse_upper:
	SUBW2	%r3,%r7
#-------------------------------------------------
.sub_norm:				# normalize the result if one
					# of the operands were normalized
	TSTW	normal(%fp)
	je	.sub_sign
	orw3	%r7,%r8,%r0
	je	.sub_zero
	jmp	.norm_end
.norm_loop:
	llsw2	&1,%r7			# high word <<= 1
	extzv	&31,&1,%r8,%r0		# highest bit of the low word
	orw2	%r0,%r7			# is inserted in high word
	llsw2	&1,%r8			# low word <<= 1
	DECW	%r5			# exponent--
.norm_end:
	bitw	&I_SET,%r7
	jz	.norm_loop
#-------------------------------------------------
.sub_sign:
	CLRW	%r4		# sign = 0
	TSTW	0(%ap)		# if operand 1 is negative
	jge	.sub_sign2
	MCOMW	%r4,%r4		# sign = !sign
.sub_sign2:
	TSTW	switch(%fp)	# if operands were switched
	je	.round
	MCOMW	%r4,%r4		# sign = !sign
	jmp	.round
#-------------------------------------------------
.sub_zero:
	movw	&0,%r5
	movw	&0,%r7
	movw	&0,%r8
	jmp	.sub_sign
#----------------------- end of magnitude subtraction ------------------------
# special cases. The shared code is in the file "dspecial.s"

	.globl	_type.double
	.globl	_invalid.double
	.globl	_qnan1.double
	.globl	_qnan2.double
	.globl	_inf1.double
	.globl	_inf2.double
#-------------------------------------------------------------------
	.data
.special_table:
	.word	_invalid.double		# arg1 is a trapping NaN
	.word	_invalid.double		# arg2 is a trapping NaN
	.word	_qnan1.double		# arg1 is a quiet NaN
	.word	_qnan2.double		# arg2 is a quiet NaN
	.word	.inf12			# both arguments are infinities
	.word	_inf1.double		# arg1 is infinity
	.word	.inf2.double		# arg2 is infinity
#-----------------------------------------------------------------------
	.text
.nan_inf:
	cmpw	op_type(%fp),&FP_ADD	# if (op type == ADD)
	je	.nan_inf3
	xorw2	&SIGN_BIT,8(%ap)	 # else restore the arg2 flag
.nan_inf3:
	JSB	_type.double		# find the type of the operands
	llsw2	&2,%r0			# index *= 4
	movw	op_type(%fp),%r1
	jmp	*.special_table(%r0)	# jump to appropiate code
#---------------------------------------------------------------------------
.inf12:					# both arguments are infinity
	movw	op_type(%fp),%r1
	cmpw	%r1,&FP_ADD		# if op == ADD
	jne	.sub_inf
	xorw3	0(%ap),8(%ap),%r0	# && they have different sign
	jnneg	.ret_arg1
.invalid:				# we have an invalid operation
	jmp	_invalid.double		# share the code
.sub_inf:
	cmpw	%r1,&FP_SUB		#if (op == SUB 
	jne	.ret_arg1
	xorw3	0(%ap),8(%ap),%r0	# && they have same sign)
	TSTW	%r0			# 
	jge	.invalid		#	invalid operation
#---------------------------------------
.ret_arg1:
	movw	0(%ap),%r0
	movw	4(%ap),%r1
.ret6:
	ret	&6
#---------------------------------------
.inf2.double:
	cmpw	op_type(%fp),&FP_ADD	# if ( operation == ADD)
	je	.ret_arg2		#	return infinity of same sign
	xorw2	&SIGN_BIT,8(%ap)	# else need to infinity of opposite sign
#---------------------------------------
.ret_arg2:
	movw	8(%ap),%r0
	movw	12(%ap),%r1
	jmp	.ret6
#------------------------------------------------------------------------------
.z_d_1:
	orw3	%r3,4(%ap),%r0		# if (arg1 == 0.0)
	je	.zero1
					# else	number is denormal
	llsw2	&11,%r3			# frac1 high <<= 11
	movw	4(%ap),%r4		# get frac1 low
	extzv	&21,&11,%r4,%r0		# get 10 high bits out of frac1 low
	orw2	%r0,%r3			# frac1 high |= low bits
	llsw2	&11,%r4
	jmp	.denorm1
.zero1:
# if argument 2 is also zero, we have to return a zero with right sign.
	orw3	%r7,12(%ap),%r0
	andw2	&0x7fffffff,%r0		# if (arg2 != 0.0)
	jne	.ret_arg2		#	return arg2
					# else
	xorw3	0(%ap),8(%ap),%r0	#  if ( the signs does not differ)
	je	.ret_arg1		#	return arg1
	.set	RC_OFFSET,22		# offset for round control in _asr
	.set	TO_MINUS,0x2		# round to zero
	extzv	&RC_OFFSET,&2,_asr,%r0	# else ( get rounding mode)
	cmpw	%r0,&TO_MINUS
	je	.ret_minus_zero		# if (rounding mode!=RM)
	movw	&0,%r0			# 	return +0.0
	movw	&0,%r1
	jmp	.ret6
.ret_minus_zero:
	movw	&SIGN_BIT,%r0		# else	return -0.0
	movw	&0,%r1
	jmp	.ret6
#-----------------------------------------------------------
.z_d_2:
	orw3	%r7,12(%ap),%r0		# if (arg2 == 0.0)
	je	.ret_arg1		#	return arg1

	llsw2	&11,%r7			# frac2 high <<= 10
	movw	12(%ap),%r8		# get frac2 low
	extzv	&21,&11,%r8,%r0		# get 10 high bits out of frac2 low
	orw2	%r0,%r7			# frac2 high |= low bits
	llsw2	&11,%r8
	jmp	.denorm2
#-----------------------------------------------------------------------------
	.def	.ef;	.val	.;	.scl	101;	.line	5;	.endef
	.def	_faddd;	.val	.;	.scl	-1;	.endef
##############################################################################
#	double _fsubd(arg1,arg2)
#	double arg1,arg2;
#	/* Subtraction of two double precision IEEE numbers */
	.text
	.align	4
	.def	_fsubd;	.val	_fsubd;	.scl	2;	.type	047;	.endef
	.globl	_fsubd
_fsubd:
	MCOUNT
	TSTW	_fp_hw
	je	.sub_soft
#-----------------------------------------------------------------------------
# MAU code
	mfsubd2	8(%ap),0(%ap)
	movw	4(%ap),%r1
	movw	0(%ap),%r0
	RET
#-----------------------------------------------------------------------------
	.align	4
.sub_soft:
	save	&6
	addw2	&local_vars,%sp		# place for temporaries
	movw	&FP_SUB,op_type(%fp)	# rememeber type of operation
	xorw2	&SIGN_BIT,8(%ap)	# invert the sign of operand 2
	jmp	.sub_entry

	.def	_fsubd;	.val	.;	.scl	-1;	.endef


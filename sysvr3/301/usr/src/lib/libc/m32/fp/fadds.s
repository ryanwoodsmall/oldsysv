#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fadds.s"
.ident	"@(#)libc-m32:fp/fadds.s	1.5.1.1"

#	float _fadds(srcF1,srcF2)
#	float srcF1,srcF2;
#
	.data
	.set	FP_ADD,0x0		# type of operation = ADD
	.set	FP_SUB,0x1		# type of operation = SUB
	.set	FORMAT,0x03030303	# format string for rounding
					# exceptions
#
#	Format of packed singles:
#
#	  31  30     23  22			      0
#	+---+----------+-------------------------------+
#	| S | exponent |	mantissa = 23 bits     |
#	+---+----------+-------------------------------+
#
	.set	sign_bit,0x80000000	# to check the sign
	.set	sign_mask,0x7fffffff	# to check bits other than sign
	.set	max_exp,0xff		# the reserved value for NaN and Inf
	.set	exp_offset,23		# for extracting the exponents
	.set	exp_width,8		#	" "
	.set	mant_width,23		# for extracting the mantissa

#	The mantissas are unpacked as shown:
#
#	  31  28  27  26  25		     3   2   1	 0 bit positions
#	+--------+---+---+-----------------+---+---+---+---+
#	| unused | V | I |   bits 22 to 1  | L | G | R | S |
#	+--------+---+---+-----------------+---+---+---+---+
#		   ^   ^		     ^   ^   ^	 ^
#		   |   |		     |   |   |   |
#	  	   |   |		     |   |   |   + sticky bit
#	  	   |   |		     |   |   +-	round bit
#	  	   |   |		     |   +-----	guard bit
#	  	   |   |		     +--------- least significant bit(0)
#	  	   |   |
#	  	   |   +-------------------------------	implicit 1 bit
#	  	   +-----------------------------------	place for overflow
#
	.set	implicit_1,0x4000000
	.set	overflow_bit,0x8000000
	.set	max_shift,25		# more than this shift makes
					# the operand mantissa become sticky
					# by denormalization
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

# The following are the local variables, allocated on the stack and
# referenced through the frame pointer.

	.set	op_type,0		# to remember the type of operation

	.set	local_vars,op_type+4	# total size of local variables
##############################################################################
	.text
	.align	4
	.def	_fadds;	.val	_fadds;	.scl	2;	.type	046;	.endef
	.globl	_fadds
_fadds:
	MCOUNT
	TSTW	_fp_hw
	je	.software
#-----------------------------------------
# MAU is present in the system.
	mfadds2	4(%ap),0(%ap)		# Execute mau instructions.
	movw	0(%ap),%r0
	RET
#-----------------------------------------
.nan_inf:				# These labels optimize
	jmp	.nan_infinity		# branches in main line code
.zero_denorm1:
	jmp	.zero_denormal1
.zero_denorm2:
	jmp	.zero_denormal2
#-----------------------------------------
	.align	4
.software:
	save	&6
	addw2	&local_vars,%sp			# place for temporaries
	movw	&FP_ADD,op_type(%fp)		# remember operation is add
.sub_entry:
	movw	0(%ap),%r0			# get argument 1
	extzv	&exp_offset,&exp_width,%r0,%r6	# exatract the exponent
	cmpw	%r6,&max_exp			# if (exp1==7f)
	je	.nan_inf			# 	arg1 is infinity/nan

	movw	4(%ap),%r1			# get argument 2
	extzv	&exp_offset,&exp_width,%r1,%r5	# extract the exponent
	cmpw	%r5,&max_exp			# if (exp2==7f)
	je	.nan_inf			#	arg2 is infinity/nan
#----------------------------------------------------------------------------
	movw	&1,%r3			# set normalize flag
#----------------------------------------------------------------------------
	extzv	&0,&mant_width,%r0,%r8	# get argument 1 mantissa
	llsw2	&3,%r8			# three bits for guard, round & sticky
	extzv	&0,&mant_width,%r1,%r7	# get argument 2 mantissa
	llsw2	&3,%r7			# three bits for guard, round & sticky
#----------------------------------------------------------------------------
	TSTW	%r6
	je	.zero_denorm1		# argument 1 is zero or denormalized
	orw2	&implicit_1,%r8		# put in the implicit 1 bit
.normalized1:
#----------------------------------------------------------------------------
	TSTW	%r5
	je	.zero_denorm2		# argument 2 is zero or denormalized
	orw2	&implicit_1,%r7		# put in the implicit 1 bit
.normalized2:
#----------------------------------------------------------------------------
	movw	&0,%r4			# clear	switch flag
#----------------------------------------------------------------------------
	subw3	%r5,%r6,%r2		# shift = exp1 - exp2
	jpos	.check_alignment	# if (exp1 > exp2) align
	je	.align_end		# if (exp1==exp2)
					#	bypass the alignment
					# else  exp2 > exp1
.switch:				#	switch the operands
	mcomw	%r4,%r4			# set switch flag
	mnegw	%r2,%r2			# shift = -shift

	movw	%r8,%r0			# and switch the operand
	movw	%r7,%r8			# mantissas so that only
	movw	%r0,%r7			# operand 2 is denormalized

	movw	%r5,%r6			# tentative result exponent=exp2
#---------------------------------------------------------------------------
.check_alignment:
	cmpw	%r2,&max_shift		# if (shift needed <= maximum shift)
	jle	.alignment1		#	align operand2
#---------------------------------------# else	try to bypass operation
					# note: we have to do something
					# if (shift > 32). Might as well
					# optimize that to (shift>=24).

	TSTW	%r0			# if (bits lost !=0)
	jne	.sticky_op		#	check the operation
	xorw3	0(%ap),4(%ap),%r0	# else  if (arg1 sign == arg2 sign)
	jge	.add_sign		#		bypass addition
	jmp	.sub_sign		#	else	bypass subtraction
.sticky_op:
	xorw3	0(%ap),4(%ap),%r0	# else  if (arg1 sign == arg2 sign)
	jge	.add_sticky		#		bypass addition
	movw	&sticky,%r7		#	else	op2 is sticky
	jmp	.sub_magnitude		#		do the subtract
.add_sticky:
	orw2	&sticky,%r8		# just set sticky
	jmp	.add_sign		# then calculate sign
#---------------------------------------------------------------------------
	.align	4
.alignment1:

#	The IS25  instruction:	extzv	offset,width,src,dest becomes
#	32000  instruction:	EXTFW	width-1,offset,src,dest
#	syntax for		INSFW	width-1,offest,src,dest
#	%r2 = shift needed
 
	subw3	&1,%r2,%r1		# %r1 = (shift needed - 1)
	EXTFW	%r1,&0,%r7,%r0		# get the bits to be shifted out
	lrsw2	%r2,%r7			# mantissa 2 >> shiftcount
	TSTW	%r0			# if (bits lost == 0)
	je	.align_end		#	alignment is done
	orw2	&1,%r7			# else	put in sticky

.align_end:
	xorw3	0(%ap),4(%ap),%r0	# are the signs same?
	jneg	.sub_magnitude		# no, we need to subtract

#--------------------------- magnitde addition ------------------------------
.add_magnitude:
	addw2	%r7,%r8			# add the mantissas
	bitw	&overflow_bit,%r8	# has overflow been set?
	je	.add_sign
					# we have one bit overflow
	andw3	&sticky,%r8,%r0		# get the bit to be lost
	lrsw2	&1,%r8			# mantissa >>= 1
	orw2	%r0,%r8			# mantissa |= sticky
	INCW	%r6			# exponent++
#------------------------- end of magnitde addition -----------------------
	.align	4
.add_sign:
	andw3	&sign_bit,0(%ap),%r0	# result sign = arg1 sign
#----------------------------------------------------------------------------
#	now format as expected in the rounding code
	.globl	_round.single
#	%r0	aligned sign
#	%r6	exponent
#	%r8	mantissa
#	%r7	RS bits
#	%r5	exception format string
#	%r3	type of operation

.round:
	andw3	&GRS,%r8,%r7		# get the GRS bits
	lrsw2	&3,%r8			# mantissa >>= 3
	andw3	&sticky,%r7,%r1		# take the sticky bit
	lrsw2	&1,%r7			# GRS >> 1
	orw2	%r1,%r7			# put the sticky back in. %r7 = RS
	movw	&FORMAT,%r5
	movw	op_type(%fp),%r3
	cmpw	%r3,&FP_ADD		# if (operation==subtract)
	jne	.restore_op2_sign	#	restore the sign
	jmp	_round.single		# else	jump to rounding code
.restore_op2_sign:
	xorw2	&sign_bit,4(%ap)	# restore sign
	jmp	_round.single
#-----------------------------------------------------------------------------
	.align	4
.sub_magnitude:
	cmpw	%r8,%r7			# if (mant1 < mant2)
	jl	.reverse		#	do (mant2 - mant1)
	subw2	%r7,%r8			# mant = mant1 - mant2
	jmp	.sub_norm
.reverse:
	mcomw	%r4,%r4			# complement switch
	subw3	%r8,%r7,%r8		# mant = mant2 - mant1
#-----------------------------------------------------------------------------
.sub_norm:				# now normalize the result
	TSTW	%r3			# if (!normalize)
	je	.sub_sign		#	calculate sign
					# else	normalize the result
	TSTW	%r8			# if (frac==0)
	je	.sub_zero
	jmp	.norm_end

.norm_loop:				# do
	llsw2	&1,%r8			#	mantissa << 1
	DECW	%r6			#	exp--
.norm_end:
	bitw	&implicit_1,%r8		# while (implicit bit==0)
	je	.norm_loop
#-----------------------------------------------------------------------------
.sub_sign:
	andw3	&sign_bit,0(%ap),%r0	# result sign = arg1 sign
	TSTW	%r4			# if (!switch)
	je	.round			#	that is the right sign
	mcomw	%r0,%r0			# else	sign = !sign
	andw2	&sign_bit,%r0		#	mask off low bits
	jmp	.round
#-----------------------------------------------------------------------------
.sub_zero:
	movw	&0,%r6			# exp = 0
	jmp	.sub_sign
#----------------------- end of magnitude subtraction ------------------------
	.align	4
.zero_denormal1:
	TSTW	%r8			# if (arg1 == 0.0)
	je	.zero1			#	check arg2
					# else	arg1 is denormal.
	llsw2	&1,%r8			# mantissa1 << 1
	jmp	.normalized1
#-----------------------------------------------------------------------------
.zero1:
	andw3	&sign_mask,%r1,%r2	# if (arg2!=0)
	jne	.ret_arg2
	xorw3	0(%ap),4(%ap),%r2	# if (signs does not differ)
	je	.ret_arg1		#	return arg1
					# else	depends upon rounding mode
	.set	rc_offset,22		# offset of round control in _asr
	.set	to_minus,0x2		# round to minus infinity
	extzv	&rc_offset,&2,_asr,%r2	# get round control bits
	cmpw	%r2,&to_minus		# if (round control==to_minus)
	je	.ret_minus_zero		#	return -0
	movw	&0,%r0			# else	return +0
	ret	&6
#-----------------------------------------------------------------------------
.ret_arg1:
	movw	0(%ap),%r0
	ret	&6
#-----------------------------------------------------------------------------
.ret_minus_zero:
	movw	&sign_bit,%r0
	ret	&6
#-----------------------------------------------------------------------------
.ret_arg2:
	movw	4(%ap),%r0
	ret	&6
#-----------------------------------------------------------------------------
	.align	4
.zero_denormal2:
	TSTW	%r7			# if (arg2 == 0.0)
	je	.ret_arg1		#	return arg1
					# else arg2 is denormal.
	llsw2	&1,%r7			# mantissa2 << 1

	orw3	%r8,%r7,%r0		# check if either operands is normalized
	bitw	&implicit_1,%r0		# if the I bit is set
	jne	.normalized2		#	join regular code
	movw	&0,%r3			# else	reset normalize flag
	jmp	.normalized2		#	join regular code
#-----------------------------------------------------------------------------
# special cases. The shared code is in the file "sspecial.s"
#
	.data
.special_table:
	.word	_invalid.single		# arg1 is a trapping NaN
	.word	_invalid.single		# arg2 is a trapping NaN
	.word	_qnan1.single		# arg1 is a quiet NaN
	.word	_qnan2.single		# arg2 is a quiet NaN
	.word	.inf12			# both arguments are infinities
	.word	_inf1.single		# arg1 is infinity
	.word	.inf2.single		# arg2 is infinity
#-----------------------------------------------------------------------
	.text
.nan_infinity:
	cmpw	op_type(%fp),&FP_ADD	# if (op type == ADD)
	je	.nan_inf3
	xorw2	&sign_bit,4(%ap)	# else restore the arg2 flag
.nan_inf3:
	JSB	_type.single		# find the type of the operands
	llsw2	&2,%r0			# index *= 4
	movw	op_type(%fp),%r1
	jmp	*.special_table(%r0)	# jump to appropiate code
#---------------------------------------------------------------------------
.inf12:					# both arguments are infinity
	cmpw	%r1,&FP_ADD		# (if op == ADD
	jne	.sub_inf
	xorw3	0(%ap),4(%ap),%r0	# && they have different sign)
	jnneg	.ret_arg1
#---------------------------------------------------------------------------
.invalid:				# we have an invalid operation
					# type of operation already in %r1
	jmp	_invalid.single		# share code in file "sspecial.s"
#---------------------------------------------------------------------------
.sub_inf:
	xorw3	0(%ap),4(%ap),%r0	# 
	TSTW	%r0			# if (the infinites have same sign)
	jge	.invalid		#	invalid operation
	jmp	.ret_arg1		# else	return operand 1
#-----------------------------------------------------------------------------
.inf2.single:
	cmpw	op_type(%fp),&FP_ADD	# if (op type == ADD)
	je	.ret_arg2		#	return infinity of same sign
	xorw2	&sign_bit,4(%ap)	# else  return infinity of opposite sign
	jmp	.ret_arg2
#-----------------------------------------------------------------------------
	.def	fadds;	.val	.;	.scl	-1;	.endef
##############################################################################
#	float _fsubs(arg1,arg2)
#	float arg1,arg2;
#	/* Subtraction of two single precision IEEE numbers */
	.text
	.align	4
	.def	_fsubs;	.val	_fsubs;	.scl	2;	.type	046;	.endef
	.globl	_fsubs
_fsubs:
	MCOUNT
	TSTW	_fp_hw
	je	.sub_soft
#-----------------------------------------------------------------------------
# MAU code
	mfsubs2	4(%ap),0(%ap)		# execute MAU subtract
	movw	0(%ap),%r0
	RET
#-----------------------------------------------------------------------------
	.align	4
.sub_soft:
	save	&6
	addw2	&local_vars,%sp		# place for temporaries
	movw	&FP_SUB,op_type(%fp)	# rememeber type of operation
	xorw2	&sign_bit,4(%ap)	# invert the sign of operand 2
	jmp	.sub_entry

	.def	fsubs;	.val	.;	.scl	-1;	.endef

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"sspecial.s"
.ident	"@(#)libc-m32:fp/sspecial.s	1.3.1.1"

#--------------------------------------------------------------------------
#	This subroutine returns one of the following values.
#	These cases are checked in sequence, so that if the return  value
#	is e.g. TNAN2, we know that TNAN1 is not true.
#	It destroys all registers.

	.set	TNAN1,0		# arg1 is a trapping NaN
	.set	TNAN2,1		# arg2 is a trapping NaN
	.set	QNAN1,2		# arg1 is a quiet NaN
	.set	QNAN2,3		# arg2 is a quiet NaN
	.set	INF12,4		# both arguments are infinities
	.set	INF1,5		# arg1 is infinity
	.set	INF2,6		# arg2 is infinity

	.set	EXP_OFFSET,23		# offset for extracting exponent
	.set	EXP_WIDTH,8		# width of exponent
	.set	MAX_EXP,0xff		# the reserved value for NaN and Inf
	.set	FRAC_SIZE,23		# the high word has 20 fraction bits
	.set	SIGN_BIT,0x80000000	# to get only the sign bit
	.set	S_NAN_BIT,0x400000	# for quiet/trapping nan
	.set	SINFINITY,0x7f800000	# single precision +infinity
#--------------------------------------------------------------------------
# Register usage:
#	%r3	argument 1
#	%r4	argument 1 exponent
#	%r5	argument 1 fraction, to distinguish NaN from Infinity
#
#	%r6,%r7,%r8 similarly for argument 2
#------------------------------------------------------------------------------
	.globl	_type.single
_type.single:
	movw	0(%ap),%r3			# arg1
	extzv	&EXP_OFFSET,&EXP_WIDTH,%r3,%r4	# arg1 exp
	extzv	&0,&FRAC_SIZE,%r3,%r5		# arg1 frac

	movw	4(%ap),%r6			# arg2
	extzv	&EXP_OFFSET,&EXP_WIDTH,%r6,%r7	# arg2 exp
	extzv	&0,&FRAC_SIZE,%r6,%r8		# arg2 frac

	cmpw	%r4,&MAX_EXP		# if (exp1 != MAX_EXP)
	jne	.arg2			#	check argument 2
	TSTW	%r5			# if (frac1 bits == 0)
	je	.inf1			#	arg1 is infinity
	bitw	&S_NAN_BIT,%r5		# if (frac1 has NaN bit on)
	jne	.qnan1			# 	arg1 is a quiet NaN
					# else	arg1 is trapping NaN
#----------------------------------------
	movw	&TNAN1,%r0
	RSB
#----------------------------------------
.inf1:
	cmpw	%r7,&MAX_EXP		# if (exp2 == MAX_EXP)
	je	.2special		#	check if it is NAN
.ret_inf1:
	movw	&INF1,%r0
	RSB
#----------------------------------------
.2special:
	TSTW	%r8			# if (arg2 == inf)
	je	.both_inf		#	return INF12
	jmp	.arg2			# we know that arg1 is not NAN
					# and arg2 is QNAN or TNAN
#----------------------------------------
.both_inf:
	movw	&INF12,%r0
	RSB
#----------------------------------------	
.qnan1:
	cmpw	%r7,&MAX_EXP		# if (exp2 == MAX_EXP)
	je	.2spec			#	check if it is TNAN
.ret_qnan1:
	movw	&QNAN1,%r0
	RSB
#----------------------------------------
.2spec:
	TSTW	%r8			# if (arg2 == inf)
	je	.ret_qnan1		#	return QNAN1
	bitw	%r8,&S_NAN_BIT		# if (arg2 NAN_BIT == 0 )
	je	.tnan2			#	arg2 is trapping NaN
	jmp	.ret_qnan1		# else	arg2 is quiet NaN, retuen 1
#----------------------------------------
.qnan2:
	movw	&QNAN2,%r0
	RSB
#----------------------------------------
.tnan2:
	movw	&TNAN2,%r0
	RSB
#-------------------------------------------------------------------------
# Now we have to check the argument2
.arg2:
	cmpw	%r7,&MAX_EXP		# if (exp2 != MAX_EXP)
	jne	.end			#	none of the list
	TSTW	%r8			# if (frac2 bits == 0)
	je	.inf2			#	arg2 is infinity
	bitw	&S_NAN_BIT,%r8		# if (high frac2 has NaN bit)
	jne	.qnan2			# 	arg1 is a quiet NaN
	jmp	.tnan2			# else	arg1 is trapping NaN
.inf2:
	movw	&INF2,%r0
.end:
	RSB
############################################################################
#
# The following special case code is shared by add/sub/mul/div
# The code gets here with
#	type of operation in	%r1
#	type of fault in	%r3

	.data
	.set	FP_INVLD,7		# type of exception
	.set	INVLD_STICKY,0x200	# to set invalid sticky
	.set	INVLD_MASK,0x4000	# to check for invalid mask
	.set	FP_F,3			# single precision operand(s)
	.set	FP_NULL,0		# null pointer
	.set	FP_UNION,0xc		# size of return value
.format:
	.byte	FP_F,FP_F,FP_NULL,FP_F	# for the exception handler
#---------------------------------------
	.text
	.globl	_asr
#---------------------------------------
	.globl	_invalid.single
_invalid.single:
	movw	&FP_INVLD,%r3		# type of exception
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid mask != 0)
	je	.gen_qnan		#	generate a quiet NaN
#---------------------------------------
#	The interface routine _getfltsw() is defined as
#	fp_union
#	_getfltsw(ftype, optype, format, op1, op2, rres)
#	fp_ftype	ftype	/* type of fault */
#	fp_op		optype	/* type of operation */
#	fp_union	*op1	/* operand 1 */
#	fp_union	*op2	/* operand 2 */
#	fp_union	*rres	/* rounded result */
#
	.globl	_trap.single
_trap.single:
	movaw	0(%sp),%r2		# pointer to return value
	addw2	&FP_UNION,%sp		# allocate for return value
	pushw	%r3			# push exception type
	pushw	%r1			# push type of operation
	pushaw	.format			# push address of
					# format for operands, result
	pushaw	0(%ap)			# push address of operand 1
	pushaw	4(%ap)			# push address of operand 2
	pushw	&FP_NULL		# no rounded result
	call	&6,_getfltsw		# raise exception
	movw	0(%r0),%r0		# pick up and return the result
	ret	&6
#-------------------------------------------------------------------------
.gen_qnan:				# generate the generic quiet NaN
					# as generated by the MAU
	movw	&0x7fffffff,%r0
	ret	&6
#----------------------------------------
	.globl	_qnan1.single
_qnan1.single:				# return argument 1
.ret_arg1:
	movw	0(%ap),%r0
	ret	&6
#---------------------------------------
	.globl	_qnan2.single
_qnan2.single:				# return argument 2
.ret_arg2:
	movw	4(%ap),%r0
	ret	&6
#--------------------------------------------------------------------------
	.globl	_inf1.single
_inf1.single:				# return arg1
	jmp	.ret_arg1
#-------------------------------------------------------------------------
	.globl	_inf2.single
_inf2.single:
	jmp	.ret_arg2


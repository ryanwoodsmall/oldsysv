#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"mfcmps.s"
.ident	"@(#)libc-m32:fp/fcmps.s	1.8"

#	Compare two single precision operands.
#	fp_cmp	mfcmpts(arg1,arg2)
#	double	arg1,arg2;

# Set PSW flags for +/-/0.
# Return with the following value in %r0
#	1) -1	for less
#	2)  0	for equal
#	3)  1	for greater
#	4)  2	for unordered

	.data
	.align	4
	.set	FP_CMPT,8		# type of operation:compare with trap
	.set	FP_CMP,9		# compare for equality
	.set	EXP_MASK,0x7f800000	# used the mask out sign and fraction
					# reserved value for NaN and infinity
	.set	FRAC_MASK,0x7fffff	# used to mask out sign and exponent
					# from high word of a double
	.set	NAN_BIT,0x400000	# to distinguish quiet/signalling
#-----------------------------------------------------------------------------
	.globl	_asr
	.globl	_fp_hw
#----------------------------------------------------------------------------
	.text
	.align	4
	.def	_fcmpts;	.val	_fcmpts;	.scl	2;	.type	052;	.endef
	.globl	_fcmpts
_fcmpts:
	MCOUNT
	TSTW	_fp_hw
	je	.software
#---------------------------
#	MAU code
	mfcmpts	0(%ap),4(%ap)
	RET
#------------------------------------------------------------------------------
	.align	4
.software:
	movw	&EXP_MASK,%r2

	andw3	0(%ap),%r2,%r0		# gets the exponent
	cmpw	%r2,%r0
	je	.nan_inf1		# arg 1 is infinity or NaN

.check2:
	andw3	4(%ap),%r2,%r1
	cmpw	%r2,%r1
	je	.nan_inf2		# arg 2 is infinity or NaN
.e_enter:
	TSTW	%r0
	je	.zero_denorm1		# arg1 is zero or denormal

#----------------------------------------
.regular:
	# comparison of two numbers, include regular, infinites, denormanls.

	movw	0(%ap),%r0		# get arg 1 high word in register
	movw	4(%ap),%r1		# get arg 2 high word in register
	jneg	.arg2_neg

	TSTW	%r0
	jneg	.LESSER			# since arg2 is positive & arg1 negative
#----------------------------------------
.both_positive:
	cmpw	%r0,%r1
	jneg	.LESSER
	je	.EQUAL
#----------------------------------------
.GREATER:				# arg1 > arg2
	movw	&1,%r0
	RET
#----------------------------------------
.LESSER:				# arg1 < arg2
	movw	&-1,%r0			# set sign, clear zero,return value.
	RET
#----------------------------------------
.arg2_neg:
	TSTW	%r0
	jnneg	.GREATER		# since arg2 is neg and arg1 >= 0
#----------------------------------------
.both_negative:				# both arguments are negative
	cmpw	%r1,%r0
	jneg	.LESSER
	jpos	.GREATER
#----------------------------------------
.EQUAL:					#  arg1 == arg2
	movw	&0,%r0			# the arguments are equal
	RET
#------------------------------------------------------------------------
# This code is needed to make sure that +0.0 and -0.0 compare equal.
.zero_denorm1:
	andw3	&FRAC_MASK,0(%ap),%r0	# if (fraction != 0)
	jnz	.regular		#	arg1 is denorm
					# else	arg1 is zero
					# is argument 2 also zero?
	andw3	&0x7fffffff,4(%ap),%r1	# if ( exp and fraction != 0)
	jnz	.regular		#	arg2 is nonzero
	jmp	.EQUAL			# else	both are zeros
#------------------------------------------------------------------------
.nan_inf1:
	andw3	&FRAC_MASK,0(%ap),%r1	# if (arg 1 == infinity)
	jz	.check2			#	check arg2
	jmp	.unordered		# else	unordered condition
#----------------------------------------
.nan_inf2:
	andw3	&FRAC_MASK,4(%ap),%r1	# if (arg 2 == infinity)
	jz	.regular		#	join regular
#----------------------------------------
	.data
	.set	FP_CMPT,8		# type of operation = compare
					# with trap
	.set	FP_INVLD,7		# type of exception
	.set	INVLD_STICKY,0x200	# to set invalid sticky
	.set	INVLD_MASK,0x4000	# to check for invalid mask
	.set	FP_F,3			# single precision operand(s)
	.set	FP_C,1			# return value of type fp_cmp
	.set	FP_NULL,0
	.set	FP_UNION,0xc		# size of return value
.format:
	.byte	FP_F,FP_F,FP_NULL,FP_C	# for the exception handler

	.text
.unordered:
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid mask != 0)
	je	.ret_unord		#	return unordered
					# else	raise exception
	save	&0
	addw2	&FP_UNION,%sp		# allocate for return value
	pushw	&FP_INVLD		# push exception type
	pushw	&FP_CMPT		# push type of operation
.share:
	pushaw	.format			# push format for operand, results
	pushaw	0(%ap)			# push address of operand 1
	pushaw	4(%ap)			# push address of operand 2
	pushaw	FP_NULL			# no return value
	movaw	0(%fp),%r2		# pointer to return value
	call	&6,_getfltsw		# raise exception
	RESTORE %fp
	movw	0(%r0),%r0		# pick up the result, set flags
	RET
#----------------------------------------
.ret_unord:
	movw	&2,%r0
	RET

	.def	.ef;	.val	.;	.scl	101;	.line	5;	.endef
	.def	_fcmpts;	.val	.;	.scl	-1;	.endef
##############################################################################
#	Nontrapping compare. This routine does not trap for quiet NaN's.
#	It however, traps for Trap NaNs.

	.text
	.align	4
	.def	_fcmps; .val	_fcmps; .scl	2;	.type	052;	.endef
	.globl	_fcmps
_fcmps:
	MCOUNT
	TSTW	_fp_hw
	je	.softe
#---------------------------
#	MAU code
	mfcmps	0(%ap),4(%ap)
	RET
#------------------------------------------------------------------------------
	.align	4
.softe:
	movw	&EXP_MASK,%r2

	andw3	0(%ap),%r2,%r0		# gets the exponent
	cmpw	%r2,%r0
	je	.nan_infe1		# arg 1 is infinity or NaN
.check2e:
	andw3	4(%ap),%r2,%r1
	cmpw	%r2,%r1
	je	.nan_infe2		# arg 2 is infinity or NaN
	jmp	.e_enter
#------------------------------------------------------------------------
.nan_infe1:
	andw3	&FRAC_MASK,0(%ap),%r1	# if (arg 1 == infinity)
	jz	.check2e		#	check arg2
	bitw	&NAN_BIT,0(%ap)		# if (arg1==signaling nan)
	je	.signaling		#	raise exception
	andw3	&EXP_MASK,4(%ap),%r0	# else is arg2 signalling?
	cmpw	&EXP_MASK,%r0		# argument 2 is not nan_inf
	jne	.ret_unord
	andw3	&FRAC_MASK,4(%ap),%r1	# if (arg 2 == infinity)
	jz	.ret_unord		#	return unordered
	jmp	.nan2			# else check nan2
#----------------------------------------
.nan_infe2:
	andw3	&FRAC_MASK,4(%ap),%r1	# if (arg 2 == infinity)
	jz	.regular		#	join regular
.nan2:
	bitw	&NAN_BIT,4(%ap)		# is arg2 signaling nan
	jne	.ret_unord		#	no: just unordered
#----------------------------------------
.signaling:
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid mask != 0)
	je	.ret_unord		#	return unordered
					# else	raise exception
	save	&0
	addw2	&FP_UNION,%sp		# allocate for return value
	pushw	&FP_INVLD		# push exception type
	pushw	&FP_CMP			# push type of operation
	jmp	.share

	.def	.ef;	.val	.;	.scl	101;	.line	5;	.endef
	.def	_fcmps;	.val	.;	.scl	-1;	.endef

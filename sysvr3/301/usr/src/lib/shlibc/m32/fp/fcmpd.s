#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"mfcmpd.s"
.ident	"@(#)libc-m32:fp/fcmpd.s	1.7"
#
#	fp_cmp	_fcmptd(arg1,arg2)	/* compare double precision arguments */
#	double	arg1,arg2;
#
#	Set PSW flags for +/-/0.
#	Return with the following value in %r0
#		1) -1	for less
#		2)  0	for equal
#		3)  1	for greater
#		4)  2	for unordered

	.data
	.align	4
	.set	FP_CMPT,8		# type of operation: compare with trap
	.set	FP_CMP,9		# compare for equality
	.set	FP_UNION,0xc		# size of return value of type fp_union

	.set	MAX_EXP,0x7ff00000	# reserved value for NaN and infinity
	.set	S_MASK,0x7fffffff	# used to mask out sign and exponent
					# from high word of a double
#-----------------------------------------------------------------------------
	.globl	_asr			# simulated status word
	.globl	_fp_hw			# hardware present flag
#----------------------------------------------------------------------------
	.text
	.align	4
	.def	_fcmptd; .val	_fcmptd; .scl	2;	.type	052;	.endef
	.globl	_fcmptd
#----------------------------------------------------------------------------
_fcmptd:
	MCOUNT
	TSTW	_fp_hw
	je	.software
#---------------------------
#	MAU code
	mfcmptd	0(%ap),8(%ap)
	RET
#------------------------------------------------------------------------------
	.align	4
.software:
	movw	&MAX_EXP,%r2

	andw3	0(%ap),%r2,%r0		# gets arg1 exponent
	cmpw	%r2,%r0			# if ( exp1 == MAX_EXP)
	je	.nan_inf		# 	arg1 is infinity or NaN

	andw3	8(%ap),%r2,%r1		# get arg2 exponent
	cmpw	%r2,%r1			# if ( exp2 == MAX_EXP)
	je	.nan_inf		# 	arg2 is infinity or NaN
.share:				# code from here is shared by both
				# types of compares
	TSTW	%r0
	je	.zero_denorm1		# arg1 is zero or denormal
#----------------------------------------
.regular:
	# comparison of two regular numbers
	movw	0(%ap),%r0		# get arg 1 high word in register
	movw	8(%ap),%r1		# get arg 2 high word in register
	TSTW	%r1
	jneg	.arg2_neg

	TSTW	%r0
	jneg	.LESSER			# since arg2 is positive & arg1 negative
#----------------------------------------
.both_positive:
	cmpw	%r0,%r1
	je	.cmp_low
	jneg	.LESSER
#----------------------------------------
.GREATER:				# arg1 > arg2
	movw	&1,%r0
	RET
#----------------------------------------
	.align	4
.LESSER:				# arg1 < arg2
	movw	&-1,%r0			# set sign, clear zero,return value.
	RET
#----------------------------------------
.cmp_low:				# high words are equal, compare
					# low order words
	CMPW	{uword}12(%ap),4(%ap)	# lower word compare is unsigned
	jlu	.LESSER
	je	.EQUAL
	jmp	.GREATER
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
	CMPW	{uword}4(%ap),12(%ap)	# lower word compare is unsigned
	jlu	.LESSER
	jpos	.GREATER
#----------------------------------------
.EQUAL:					#  arg1 == arg2
	movw	&0,%r0			# the arguments are equal
	RET
#------------------------------------------------------------------------
# This code is needed to make sure that +0.0 and -0.0 compare equal.
.zero_denorm1:
	andw3	&S_MASK,0(%ap),%r0
	orw2	4(%ap),%r0
	jnz	.regular
					# we know that arg1 is zero
					# is argument 2 also zero?
	andw3	&S_MASK,8(%ap),%r0
	orw2	12(%ap),%r0
	jnz	.regular
	jmp	.EQUAL			# both arguments are zero
#------------------------------------------------------------------------------
# special cases
	.data
	.set	FP_INVLD,7		# type of exception
	.set	INVLD_STICKY,0x200	# to set invalid sticky
	.set	INVLD_MASK,0x4000	# to check the invalid mask
	.set	FP_D,4			# double precision operand(s)
	.set	FP_C,1			# compare type
	.set	FP_NULL,0		# null
.format:
	.byte	FP_D,FP_D,FP_NULL,FP_C	# for the exception handler

	.set	TNAN1,0		# arg1 is a trapping NaN
	.set	TNAN2,1		# arg2 is a trapping NaN
	.set	QNAN1,2		# arg1 is a quiet NaN
	.set	QNAN2,3		# arg2 is a quiet NaN
	.set	INF12,4		# both arguments are infinities
	.set	INF1,5		# arg1 is infinity
	.set	INF2,6		# arg2 is infinity
.special_table:
	.word	.trap		# trap for arg1 == trap Nan
	.word	.trap		# trap for arg2 == trap Nan
	.word	.trap		# trap for arg1 == quiet Nan
	.word	.trap		# trap for arg2 == quiet Nan
	.word	.reg		# join regular code
	.word	.reg		# join regular code
	.word	.reg		# join regular code
#-----------------------
	.globl	_type.double		# type.double destroys registers %r3-%r8
	.text
.nan_inf:
	save	&6			# hence the need to save them
	JSB	_type.double		# find the type of the operands
	llsw2	&2,%r0			# index *= 4
	jmp	*.special_table(%r0)	# jump to appropiate code
#----------------------------------------
.trap:
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid mask != 0)
	je	.unordered		#	unordered condition
					# else	raise_exception
	movaw	0(%sp),%r2		# pointer to return value
	addw2	&FP_UNION,%sp		# allocate space for return value
	pushw	&FP_INVLD		# push exception type
	pushw	&FP_CMPT		# push type of operation
.trap2:		# the rest of the code is shared by the other compare
	pushaw	.format			# push format for operand, results
	pushaw	0(%ap)			# push address of operand 1
	pushaw	8(%ap)			# push address of operand 2
	pushw	&FP_NULL		# no rounded result
	call	&6,_getfltsw		# raise exception
	movw	0(%r0),%r0		# get the return value
	movaw	0(%fp),%sp		# RESTORE %r3
	POPW	%r8
	POPW	%r7
	POPW	%r6
	POPW	%r5
	POPW	%r4
	POPW	%r3
	POPW	%fp
	TSTW	%r0			# set flags according to return value
	RET
#----------------------------------------
.unordered:
	movaw	0(%fp),%sp		 # RESTORE %r3
	POPW	%r8
	POPW	%r7
	POPW	%r6
	POPW	%r5
	POPW	%r4
	POPW	%r3
	POPW	%fp
	movw	&2,%r0			# set flags to show unordered
	RET
#----------------------------------------
.reg:
	movaw	0(%fp),%sp		 # RESTORE %r3
	POPW	%r8
	POPW	%r7
	POPW	%r6
	POPW	%r5
	POPW	%r4
	POPW	%r3
	POPW	%fp
	jmp	.regular
#-----------------------------------------------------------------------------
	.def	_fcmptd;	.val	.;	.scl	-1;	.endef
################################################################################
#	fp_cmp	_fcmpd(arg1,arg2)	/* compare double precision arguments */
#	double	arg1,arg2;		/* this compare does not trap for
#					   quiet nans  */
#------------------------------------------------------------------------------
	.text
	.align	4
.software2:
	movw	&MAX_EXP,%r2
	andw3	0(%ap),%r2,%r0		# gets arg1 exponent
	cmpw	%r2,%r0			# if ( exp1 == MAX_EXP)
	je	.nan_inf2		# 	arg1 is infinity or NaN

	andw3	8(%ap),%r2,%r1		# get arg2 exponent
	cmpw	%r2,%r1			# if ( exp2 == MAX_EXP)
	jne	.share			# 	arg2 is infinity or NaN
#------------------------------------------------------------------------------
	.data
.special_table2:
	.word	.tnan			# trap for arg1 == trap NaN
	.word	.tnan			# trap for arg2 == trap NaN
	.word	.unordered		# unordered condition for quiet NaN
	.word	.unordered		# unordered condition for quiet NaN
	.word	.reg			# join regular code
	.word	.reg			# join regular code
	.word	.reg			# join regular code
#-----------------------
	.text
.nan_inf2:
	save	&6			# hence the need to save them
	JSB	_type.double		# find the type of the operands
	llsw2	&2,%r0			# index *= 4
	jmp	*.special_table2(%r0)	# jump to appropiate code
#----------------------------------------
.tnan:					# 
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid mask != 0)
	je	.unordered		#	unordered condition
					# else	raise_exception
	movaw	0(%sp),%r2		# pointer to return value
	addw2	&FP_UNION,%sp		# allocate space for return value
	pushw	&FP_INVLD		# push exception type
	pushw	&FP_CMP			# push type of operation
	jmp	.trap2			# share the rest of the code
	
#------------------------------------------------------------------------------
	.align	4
	.def	_fcmpd; .val	_fcmpd; .scl	2;	.type	052;	.endef
	.globl	_fcmpd
_fcmpd:
	MCOUNT
	TSTW	_fp_hw
	je	.software2
#---------------------------
#	MAU code
	mfcmpd	0(%ap),8(%ap)
	RET
#------------------------------------------------------------------------------
	.def	_fcmpd;	.val	.;	.scl	-1;	.endef


#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"ftstou.s"
.ident	"@(#)libc-m32:fp/ftstou.s	1.4.1.4"
#****************************************************************************
#	unsigned long integer _ftstol(srcF)
#	float	srcF;
#	/* Truncate single precision floating to unsigned long integer */
#	/* Generates integer overflow if the source is negative */
#****************************************************************************
	.data
	.set	SEXP_OFFSET,23		# exponent starts at bit 24
	.set	SEXP_WIDTH,8		# size of double exponent
	.set	SEXP_BIAS,127		# bias for double exponent
	.set	SEXP_MAX,255		# resereved  exponent value
					# for NaN and infinity
	.set	HIDDEN_BIT,0x800000	# the implied highest bit in fraction
	.set	SFRAC_MASK,0x7fffff	# mask to clear exp and sign from frac
	.set	MAX_SHIFT,32		# maximum number of shifts necessary
#****************************************************************************
	.text
	.align	4
	.def	_ftstou; .val	_ftstou; .scl	2;	.type	056;	.endef
	.globl	_ftstou
#-----------------------------------------------------------------------------
_ftstou:
	MCOUNT
	TSTW	_fp_hw			# if ( hardware present)
	je	.software		#	no, emulate
#----------------------------------
	.set	FP_RZ,0x3
	.set	CSC_BIT,0x20000	# context switch control bit in ASR
				# is manually set after a "mmovta"
				# so that the new ASR is not lost on switch
	save	&0
	addw2	&4,%sp
	mmovfa	0(%fp)			# get asr
	extzv	&22,&2,0(%fp),%r2	# save rounding mode
	insv	&FP_RZ,&22,&2,0(%fp)	# change rounding mode
	orw2	&CSC_BIT,0(%fp)		# make sure this ASR is saved
	mmovta	0(%fp)			# write asr
	mmovsu	0(%ap),0(%ap)		# do the convert
	movw	0(%ap),%r0		# need result in %r0
	mmovfa	0(%fp)			# get asr
	insv	%r2,&22,&2,0(%fp)	# restore rounding mode
	orw2	&CSC_BIT,0(%fp)		# make sure this ASR is saved
	mmovta	0(%fp)			# write asr back
	ret	&0
#-----------------------------------------------------------------------------
.overflow:				# force an integer overflow
	mulw2	%r0,%r0
	RET
#----------------------------------------------------------------------------
	.align	4
.software:
	movw	0(%ap),%r0
	jneg	.zero			# return integer zero
	jz	.zero			# if ( arg == 0 ) return zero
#----------------------------------------
	extzv	&SEXP_OFFSET,&SEXP_WIDTH,%r0,%r2 # extract the exponent
	cmpw	&SEXP_MAX,%r2		# if (exp == reserved value)
	je	.nan_inf		#	raise exception

	subw2	&SEXP_BIAS,%r2		# exponent = exp - BIAS
	jneg	.zero			# if (exponent < 0)

	andw2	&SFRAC_MASK,%r0		# mask off exponent from number
	orw2	&HIDDEN_BIT,%r0		# insert hidden bit

	llsw2	&8,%r0			# align the bits on 31st bit

	subw3	%r2,&MAX_SHIFT-1,%r2	# shift needed = 31 - exponent
	jneg	.overflow		# if ( shift > MAX_SHIFT) overflow
	lrsw2	%r2,%r0

	RET
#-------------------------------------------------------------
.zero:					# return integer zero
	movw	&0,%r0
	RET
#----------------------------------------------------------------------------
	.data
	.set	FP_CONV,6		# type of operation
	.set	FP_INVLD,7		# type of exception
	.set	INVLD_STICKY,0x200	# invalid sticky bit in _asr
	.set	INVLD_MASK,0x4000	# invalid mask bit in _asr
	.set	FP_F,3			# single precision source
	.set	FP_U,7			# unsigned long integer result
	.set	FP_NULL,0
	.set	FP_UNION,64		# size of return value
.format:
	.byte	FP_F,FP_NULL,FP_NULL,FP_U # for the exception handler
#----------------------------------------------------------------------------
	.text
.nan_inf:				# in either case we have
					# invalid operation exception
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid mask == 0)
	je	.undefined		#	result is undefined
.exception:
	save	&0
	movaw	0(%sp),%r2		# address of return value
	addw2	&FP_UNION,%sp		# allocate for return value
	pushw	&FP_INVLD		# type of exception
	pushw	&FP_CONV		# type of operation
	pushaw	.format			# format for operands etc.
	pushaw	0(%ap)			# address of operand
	pushw	&FP_NULL		# no second argument
	pushw	&FP_NULL		# no rounded result
	call	&6,_getfltsw		# raise exception
	movw	0(%r0),%r0		# pull out the result
	ret	&0
#----------------------------------------------------------------------------
.undefined:
	RET
#----------------------------------------------------------------------------
	.def	_ftstou;	.val	.;	.scl	-1;	.endef

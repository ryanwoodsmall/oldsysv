#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fstod.s"
	.ident	"@(#)libc-m32:fp/fstod.s	1.4.2.1"

#	convert single precision floating to double precision floating.
#	double	_fstod(srcF)
#	float	srcF;

	.data
	.set	DEXP_OFFSET,20		# offset for double exponent
	.set	DEXP_WIDTH,11		# size of double exponent
	.set	DEXP_BIAS,1023		# bias for double exponent
	.set	DINF_EXP,0x7ff00000	# reserved exponent for NaN and Inf
					# (aligned at its place)
	.set	DNAN_BIT,0x800000	# for quiet nans

	.set	SEXP_OFFSET,23		# offset for single exponent
	.set	SEXP_WIDTH,8		# size of single exponent
	.set	SEXP_BIAS,127		# bias for single exponent
	.set	SINF_EXP,255		# reserved exponent for NaN and Inf
	.set	SFRAC_MASK,0x7fffff	# this mask get the fraction bits
	.set	SIGN_BIT,0x80000000	# to put the sign bit
	.set	SIGN_MASK,0x7fffffff	# to mask off the sign bit
	.set	I_BIT,0x800000		# implied bit of single fraction
	.set	SNAN_BIT,0x400000	# to distinguish trapping NaN from
					# quiet NaN

#	Register usage:
#	%r0	scratch
#	%r1	integer
#	%r2	exponent
#-----------------------------------------------------------------------------
	.text
	.align	4
	.def	_fstod;	.val	_fstod;	.scl	2;	.type	047;	.endef
	.globl	_fstod
#-----------------------------------------------------------------------------
_fstod:
	MCOUNT
	TSTW	_fp_hw			# if (hardware present)
	je	.software		#	no, emulate
#----------------------------------
.mau:
	save	&0
	addw2	&8,%sp			# allocate stack space
	mmovsd	0(%ap),0(%fp)		# do conversion
	movw	4(%fp),%r1		# return result in %r0,%r1
	movw	0(%fp),%r0
	ret	&0
#----------------------------------------------------------------------------
.zero:					# return signed zero
	movw	%r1,%r0			# take the sign from arg
	movw	&0,%r1			# zero second word
	RET
#----------------------------------------------------------------------------
	.align	4
.software:
	movw	0(%ap),%r1
	andw3	&SIGN_MASK,%r1,%r0
	je	.zero			# if ( arg == 0 ) return zero
	extzv	&SEXP_OFFSET,&SEXP_WIDTH,%r1,%r2 # get exponent
	je	.denorm			# if (exp == MIN_EXP)
	cmpw	%r2,&SINF_EXP		# if (exp == MAX_EXP)
	je	.inf_nan

	addw2	&DEXP_BIAS-SEXP_BIAS,%r2# adjust the exponent for
.pack:					# double precision
	andw3	&SIGN_BIT,%r1,%r0	# put the sign in
	llsw2	&DEXP_OFFSET,%r2	# align the exponent
	orw2	%r2,%r0			# OR in the exponent
	extzv	&3,&20,%r1,%r2		# take the high 20 bits of fraction
	orw2	%r2,%r0			# and OR in
	llsw2	&29,%r1			# align the 3 low bits of fraction
	RET
#---------------------------------------------------------------------------
.denorm:
	movw	&DEXP_BIAS-SEXP_BIAS+1,%r2# adjust the exponent for double
					# the 1 is for the fact that the
					# source is denorm, result is normal
.norm_loop:
	bitw	&I_BIT,%r1		# while ( frac & I_BIT != 1 ) {
	jnz	.pack
	pushw	%r8			#	need little more storage
	andw3	&SIGN_BIT,%r1,%r8	#	remember sign
	llsw2	&1,%r1			# 	frac <<= 1
	orw2	%r8,%r1			#	put the sign back
	POPW	%r8			#	restore %r8
	subw2	&1,%r2			# 	exp--
	jmp	.norm_loop		# }
#--------------------------------------------------------------------------
.inf_nan:
	andw3	&0x80000000,%r1,%r0	# put the sign in
	andw3	&SFRAC_MASK,%r1,%r2	# if ( frac != 0)
	jnz	.nan			#	number is NaN
					# else	number is infinity
	orw2	&DINF_EXP,%r0		# infinity in double precision
	movw	&0,%r1			# second word is zero
	RET

.nan:
	bitw	&SNAN_BIT,%r1		# if (NaN bit == 0)
	je	.tnan			#	Trapping NaN

.qnan:					# else	quiet NaN
	orw2	&DINF_EXP,%r0		# put in reserved exponent (old sign)
	extzv	&3,&20,%r1,%r2		# top 20 bits of qnan mantissa
	orw2	%r2,%r0			# put them in the dest mantissa
	llsw2	&29,%r1			# lowest 3 bits of qnan mantissa
	RET
#--------------------------------------------------------------------------
.tnan:
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid trap == enabled)
	jnz	.raise_exception	#	raise exception
					# else	generate quiet NaN
	movw	&0x7fffffff,%r0		# sign = 0, exp = 7f, mant= all 1's
	movw	&0xffffffff,%r1
	RET
#-------------------------------------------------------------------------
	.data
	.set	FP_INVLD,7		# type of exception
	.set	FP_CONV,6		# type of operation
	.set	INVLD_STICKY,0x200	# to set invalid sticky
	.set	INVLD_MASK,0x4000	# to check for invalid mask
	.set	FP_F,3			# source is single
	.set	FP_D,4			# double precision result
	.set	FP_NULL,0
	.set	FP_UNION,0xc		# size of return value fp_union
.format:
	.byte	FP_F,FP_NULL,FP_NULL,FP_D
	.text
#-------------------------------------------------------------------------
.raise_exception:
	save	&0
	movaw	0(%sp),%r2		# pointer to return value
	addw2	&FP_UNION,%sp		# space for return value

	pushw	&FP_INVLD		# push exception type
	pushw	&FP_CONV		# push type of operation
	pushaw	.format			# push format for operand, results
	pushaw	0(%ap)			# push address of operand 1
	pushw	&FP_NULL		# no second operand
	pushaw	0(%fp)			# push address of return value
	call	&6,_getfltsw		# raise exception
	movw	4(%r0),%r1		# return the result
	movw	0(%r0),%r0
	ret	&0

	.def	_fstod;	.val	.;	.scl	-1;	.endef


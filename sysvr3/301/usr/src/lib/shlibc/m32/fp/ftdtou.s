#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"ftdtou.s"
.ident	"@(#)libc-m32:fp/ftdtou.s	1.5.1.4"
#****************************************************************************
#	unsigned long integer _ftdtou(srcD)
#	double	srcD;
#	/* Truncate double precision floating to unsigned long integer */
#	/* Generates integer overflow if the source is negative */
#****************************************************************************
	.data
	.set	DEXP_OFFSET,20		# exponent starts at bit 21
	.set	DEXP_WIDTH,11		# size of double exponent
	.set	DEXP_BIAS,1023		# bias for double exponent
	.set	DEXP_MAX,2047		# resereved  exponent value
					# for NaN and infinity
	.set	SIGN_BIT,0x80000000	# to get the sign bit
	.set	N_SIGN,0x7fffffff	# mask to clear out the sign
	.set	HIDDEN_BIT,0x100000	# the implied highest bit in fraction
	.set	DFRAC_MASK,0xfffff	# mask to clear exp and sign from frac
	.set	MAX_SHIFT,32		# maximum number of shifts necessary

	.set	MAX_NEG_DI,0xc1e00000	# This is the double equivalent
					# of maximum negative number.
	.set	MAX_NEG_I,0x80000000	# this is the hex form
					# (Decimal -2147483648)

#	Register usage:
#	%r0	scratch
#	%r1	high part of double source
#	%r2	low  part of double source
#-----------------------------------------------------------------------------
	.text
	.align	4
	.def	_ftdtou; .val	_ftdtou; .scl	2;	.type	056;	.endef
	.globl	_ftdtou
#-----------------------------------------------------------------------------
_ftdtou:
	MCOUNT
	TSTW	_fp_hw			# if (hardware present)
	je	.software		#	no, emulate
#----------------------------------
	.set	FP_RZ,0x3		# rounding mode = truncate
	.set	CSC_BIT,0x20000	# context switch control bit in ASR
				# is manually set after a "mmovta"
				# so that the new ASR is not lost on switch
.mau:
	save	&0
	addw2	&4,%sp
	mmovfa	0(%fp)			# get asr
	extzv	&22,&2,0(%fp),%r2	# save rounding mode
	insv	&FP_RZ,&22,&2,0(%fp)	# change rounding mode
	orw2	&CSC_BIT,0(%fp)		# make sure this ASR is saved
	mmovta	0(%fp)			# write asr
	mmovdu	0(%ap),0(%ap)		# do the convert
	movw	0(%ap),%r0		# need result in %r0
	mmovfa	0(%fp)			# get asr
	insv	%r2,&22,&2,0(%fp)	# restore rounding mode
	orw2	&CSC_BIT,0(%fp)		# make sure this ASR is saved
	mmovta	0(%fp)			# write asr back
	ret	&0
#############################################################################
.zero:					# return integer zero
	movw	&0,%r0
	RET
#----------------------------------------------------------------------------
.undefined:				# try to force integer overflow
	mulw2	%r1,%r1
	RET
#----------------------------------------------------------------------------
	.align	4
.software:
	movw	0(%ap),%r1
	movw	4(%ap),%r2

	andw3	&N_SIGN,%r1,%r0		# remove the sign
	orw2	%r2,%r0			# OR of all other bits
	jz	.zero			# if ( arg == 0 ) return zero

	TSTW	%r1			# if (arg is negative)
	jneg	.zero			#	return zero

					# we know number is positive:
	extzv	&DEXP_OFFSET,&DEXP_WIDTH,%r1,%r0 # extract the exponent
	cmpw	&DEXP_MAX,%r0		# if (exp == reserved value)
	je	.nan_inf		#	raise exception

	subw2	&DEXP_BIAS,%r0		# exponent = exp - BIAS
	jneg	.zero			# if (exponent < 0)
	cmpw	%r0,&MAX_SHIFT		# if ( shift > MAX_SHIFT)
	jge	.overflow
	andw2	&DFRAC_MASK,%r1		# mask off exponent from high word
	orw2	&HIDDEN_BIT,%r1		# insert hidden bit

	llsw2	&11,%r1			# align the top 21 bits
	extzv	&21,&11,%r2,%r2		# get the low 11 bits
	orw2	%r2,%r1

	subw3	%r0,&MAX_SHIFT-1,%r2	# shift needed = 31 - exponent
	lrsw3	%r2,%r1,%r0

	RET
#----------------------------------------------------------------------------
.overflow:				# force an integer overflow
	mulw3	%r1,%r1,%r0
	RET
#----------------------------------------------------------------------------
	.data
	.set	FP_CONV,6		# type of operation
	.set	FP_INVLD,7		# type of exception
	.set	INVLD_STICKY,0x200	# to set invalid sticky
	.set	INVLD_MASK,0x4000	# to check for invalid mask
	.set	FP_D,4			# double precision source
	.set	FP_U,7			# unsigned integer result
	.set	FP_NULL,0		# null pointer
	.set	FP_UNION,0xc		# size of return value
	.set	NAN_BIT,0x8000		# to set quiet NaN
.format:
	.byte	FP_D,FP_NULL,FP_NULL,FP_U# for the exception handler
	.text
	.globl	_asr
#---------------------------------------
.nan_inf:				# in either case we have
					# invalid operation exception
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid mask != 0)
	jne	.trap			#	raise exception
	RET				# else result is undefined

.trap:
	save	&0
	movaw	0(%sp),%r2		# pointer to return value
	addw2	&FP_UNION,%sp		# allocate for return value
	pushw	&FP_INVLD		# push exception type
	pushw	&FP_CONV		# push type of operation
	pushaw	.format			# push format for operand, results
	pushaw	0(%ap)			# push address of operand 1
	pushw	&FP_NULL		# no operand 2
	pushw	&FP_NULL		# no rounded result
	call	&6,_getfltsw		# raise exception
	movw	0(%r0),%r0
	ret	&0
#-----------------------------------------------------------------------------
	.def	_ftdtou;	.val	.;	.scl	-1;	.endef


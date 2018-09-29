#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fdtos.s"
	.ident	"@(#)libc-m32:fp/fdtos.s	1.6.2.1"

#	convert double precision floating to single precision floating.
#	float	_fdtos(srcD)
#	double	srcD;

	.data
	.set	DEXP_OFFSET,20		# offset for double exponent
	.set	DEXP_WIDTH,11		# size of double exponent
	.set	DEXP_BIAS,1023		# bias for double exponent
	.set	DI_BIT,0x100000		# implied bit for double
	.set	DMAX_EXP,0x7ff		# reserved exponent for NaN and Inf
	.set	SIGN_MASK,0x7fffffff	# to mask off sign bit
	.set	SIGN_BIT,0x80000000	# to check the sign bit
	.set	D_NAN_BIT,0x80000	# to distinguish trapping NaN from
					# quiet NaN
	.set	G_S_QNAN,0x7fffffff	# generic single precision quiet NaN

	.set	SEXP_OFFSET,23		# offset for single exponent
	.set	SEXP_WIDTH,8		# size of single exponent
	.set	SEXP_BIAS,127		# bias for single exponent
	.set	SMIN_EXP,0		# reserved exponent for zero/denormal
	.set	SMAX_EXP,255		# reserved exponent for NaN and Inf
	.set	SFRAC_MASK,0x7fffff	# this mask get the fraction bits
	.set	SI_BIT,0x800000		# implied bit of single fraction
	.set	S_NAN_BIT,0x400000	# for quiet/trapping nan
	.set	OVERFLOW_BIT,0x1000000	# overflow out of implied bit
	.set	SINFINITY,0x7f800000	# single precision +infinity
	.set	SLARGE,0x7f7fffff	# largest single precision value

	.set	FP_UNION,0xc		# size of return value fp_union
#-----------------------------------------------------------------------------
	.text
	.align	4
	.def	_fdtos;	.val	_fdtos;	.scl	2;	.type	046;	.endef
	.globl	_fdtos
#-----------------------------------------------------------------------------
_fdtos:
	MCOUNT
	TSTW	_fp_hw
	je	.software
#----------------------------------
.mau:
	mmovds	0(%ap),0(%ap)
	movw	0(%ap),%r0
	RET
#----------------------------------------------------------------------------
.szero:					# return signed zero
	RET				# which already is in %r0
#----------------------------------------------------------------------------
	.align	4
.software:
	movw	0(%ap),%r0		# get argument high word
	movw	4(%ap),%r1		# get argument low  word
	andw3	&SIGN_MASK,%r0,%r2	# argument high without sign
	orw2	%r1,%r2			# OR of all bits other than sign
	je	.szero			# if ( arg == 0 ) return zero

	save	&6
	addw2	&FP_UNION+8,%sp		# space for return value & rounded result
					# in case of exception

	extzv	&0,&20,%r0,%r8		# top 20 bits of fraction
	extzv	&DEXP_OFFSET,&DEXP_WIDTH,%r0,%r6 # exponent
	cmpw	%r6,&DMAX_EXP		# if (exp == 0x7ff)
	je	.inf_nan		#	nan or infinity
	TSTW	%r6			# if (source is denormal)
	je	.no_implied		#	no implied bit
	orw2	&DI_BIT,%r8		# else	put in implied one
.no_implied:
	subw2	&DEXP_BIAS-SEXP_BIAS,%r6# adjust the exponent for
					# single precision

	llsw2	&3,%r8			# high frac << 3
	lrsw3	&29,%r1,%r7		# 3 bits from the low word
	orw2	%r7,%r8			# put the low 3 bits of fraction

	extzv	&28,&1,%r1,%r7		# R bit
	llsw2	&1,%r7 			# R bit << 1
	llsw3	&4,%r1,%r5		# 28 sticky bits = 52 - 23 - R bit
	jz	.no_sticky
	orw2	&1,%r7			# put sticky in
.no_sticky:
	andw2	&SIGN_BIT,%r0		# sign bit in place
#---------------------------------------------------------------------------
#	This routine has its own code for rounding. This is because:
#		"Trapped overflow on conversion from a binary
#		floating-point format shall deliver to the trap
#		handler a result in that or a wider format,
#		possibley with the exponent bias adusted, but
#		rounded to the destination's precision."
#	Similar treatement is needed for for trapped underflows.
#
#	For overflow, underflow, the trapped result is in double
#	precision, but rounded after 23 significant bits.
#
# SINGLE PRECISION ROUNDING
# Input:
#	%r8	fraction
#	%r7	sticky bits
#	%r6	exponent
#	%r0	sign flag
# Register usage:
#	%r4	tinyness flag
	.data
	.set	RC_OFFSET,22		# for getting the round control bits
	.set	TO_PLUS,0x1		# rounding mode = towards plus
	.set	TO_MINUS,0x2		# rounding mode = towards minus
	.set	TO_ZERO,0x3		# rounding mode = truncate
#---------------------------------------------------------------------------
	.text
	movw	&0,%r4			# clear underflow flag

	cmpw	%r6,&SMIN_EXP		# if (exp <= MIN_EXP)
	jg	.test_inexact
#---------------------------------------- denormal processing
	movw	&1,%r4			#	remember tinyness
	bitw	&UM_MASK,_asr		# if (underflow trap==enabled)
	je	.no_u_trap
	addw2	&192,%r6		# 	This prevents loss of
					#	precision bits by rounding

.no_u_trap:
	TSTW	%r6
	jpos	.test_inexact
	mnegw	%r6,%r6
	cmpw	%r6,&23			# if (-exp > 23)
	jg	.total_loss		#	denormal would produre zero
					# else denormalize
.denorm_loop:
	andw3	&1,%r7,%r2		# old S
	lrsw2	&1,%r7			# RS >> 1
	orw2	%r2,%r7			# new S = old R | old S
	andw3	&1,%r8,%r2		# new R bit
	llsw2	&1,%r2			# new R << 1
	orw2	%r2,%r7			# is ORed onto sticky
	lrsw2	&1,%r8			# fraction >> 1
	subw2	&1,%r6			# exponent - 1
	jge	.denorm_loop		# while ( exp >= 0)
					# note: denormal bias is 126, not 127
	movw	&0,%r6
	jmp	.test_inexact
#----------------------------------------
.total_loss:
	movw	&1,%r7			# sticky = 1
	movw	&0,%r8			# fraction = 0
	movw	&0,%r6			# exp = 0
#----------------------------------------
.test_inexact:
	TSTW	%r7			# if ( no sticky)
	je	.pack			#	try to pack
	orw2	&INEXACT_STICKY,_asr	# set inexact sticky
#----------------------------------------
	extzv	&RC_OFFSET,&2,_asr,%r5	# get rounding mode
	je	.to_near
	cmpw	%r5,&TO_ZERO
	je	.pack			# rounding mode = truncate
	cmpw	%r5,&TO_PLUS
	je	.to_plus
	jmp	.to_minus
#---------------------------------------------------------------------------
.to_plus:
	TSTW	%r0			# if the sign is postive
	je	.round_up		#	round up
	jmp	.pack			# else	truncate to inexact result
#---------------------------------------------------------------------------
.to_minus:
	TSTW	%r0			# if the sign is negative
	je	.pack			#	truncate result
					# else	round down. i.e.
	jmp	.round_up		# round up the negative magnitude
#---------------------------------------------------------------------------
.to_near:
	bitw	&2,%r7			# if ( R == 0 )
	je	.pack			#	inexact, no rounding
	bitw	&1,%r8			# if ( L set)
	jne	.round_up		#	round up
	bitw	&1,%r7			# if ( S ==0)
	je	.pack			#	inexact, no rounding
.round_up:
	addw2	&1,%r8			# frac = frac + 1
	bitw	&OVERFLOW_BIT,%r8	# if (overflow)
	je	.pack
	lrsw2	&1,%r8			#	frac >>1
	addw2	&1,%r6			#	exp ++
#---------------------------------------------------------------------------
.pack:
	insv	%r6,&SEXP_OFFSET,&SEXP_WIDTH,%r0
					# put in the exponent
	insv	%r8,&0,&23,%r0		# put in the fraction bits

	TSTW	%r4			# check tinyness
	jnz	.under
.pack2:
	cmpw	%r6,&SMAX_EXP		# if (exp >= MAX_EXP)
	jge	.over			#	overflow
.pack3:
	TSTW	%r7			# check inexact
	jnz	.check_inxact_trap
.ret6:
	ret	&6
#---------------------------------------------------------------------------
	.data
	.set	FP_CONV,6		# type of operation

	.set	FP_UFLW,4		# type of exception:underflow
	.set	UFLW_STICKY,0x80	# to set sticky
	.set	UM_MASK,0x1000		# to check mask

	.set	FP_OVFLW,3		# type of exception:overflow
	.set	OVFLW_STICKY,0x100	# to set sticky
	.set	OM_MASK,0x2000		# to check mask

	.set	FP_INVLD,7		# type of exception:invalid
	.set	INVLD_STICKY,0x200	# to set invalid sticky
	.set	INVLD_MASK,0x4000	# to check for invalid mask

	.set	FP_INEXACT,5		# type of exception:inexact
	.set	INEXACT_STICKY,0x40000	# to set inexact sticky
	.set	IM_MASK,0x400		# to check inexact mask

	.set	FP_F,3			# source is single
	.set	FP_D,4			# double precision result
	.set	FP_NULL,0

.format:
	.byte	FP_D,FP_NULL,FP_D,FP_F	# for under/overflow & inexact
#--------------------------------------------------------------------------
	.text
.under:
	bitw	&UM_MASK,_asr		# if ( UM == 1 )
	jne	.under_trap		#	raise underflow trap
					# otherwise underflow sticky is
					# to be set if both tiny and inexact
	TSTW	%r7			# test if inexact
	je	.ret6
	orw2	&UFLW_STICKY,_asr	# set underflow sticky
	jmp	.pack2
#--------------------------------------------------------------------------
.check_inxact_trap:
	bitw	&IM_MASK,_asr		# if (inexact trap==enabled)
	je	.ret6
					#	raise inexact trap
	pushw	&FP_INEXACT		# push exception type
	jmp	.share
#--------------------------------------------------------------------------
.over:
	orw2	&OVFLW_STICKY,_asr
	bitw	&OM_MASK,_asr		# if ( OM == 1 )
	jne	.over_trap		#	raise overflow trap
					# else depends on rounding mode
	andw3	&SIGN_BIT,0(%ap),%r2	# get sign bit
	extzv	&RC_OFFSET,&2,_asr,%r5	# get rounding mode
	je	.over_near
	cmpw	%r5,&TO_ZERO
	je	.over_zero
	cmpw	%r5,&TO_PLUS
	je	.over_plus
	jmp	.over_minus
#---------------------------------------
.over_near:				# result is signed infinity
	orw3	&SINFINITY,%r2,%r0
	jmp	.pack3
#---------------------------------------
.over_zero:				# result is signed largest finite
					# number
	orw3	&SLARGE,%r2,%r0
	jmp	.pack3
#---------------------------------------
.over_plus:
	TSTW	%r2			# if positive
	jpos	.over_near		#	+infinity
	jmp	.over_zero		# else	-largest finite
#---------------------------------------
.over_minus:
	TSTW	%r2			# if positive
	jpos	.over_zero		#	+largest finite
	jmp	.over_near		# else	-infinity
#--------------------------------------------------------------------------
.under_trap:
	orw2	&UFLW_STICKY,_asr	# set underflow sticky
	subw2	&192,%r6		# take out the exponent adjustment
	pushw	&FP_UFLW		# push exception type
#----------------------------------------
.share:
	pushw	&FP_CONV		# type of operation
	pushaw	.format			# push format for source & result
	pushaw	0(%ap)			# push address of operand 1
	pushw	&FP_NULL		# no second operand

					# change the result to
					# double precison format
	extzv	&3,&20,%r8,%r1
	insv	%r1,&0,&20,%r0		# 20 high bits of fraction
	addw2	&DEXP_BIAS-SEXP_BIAS,%r6# adjust exponent for double
	insv	%r6,&20,&11,%r0		# 11 bits of exponent
	movw	%r0,0(%fp)		# store rounded result
	llsw3	&29,%r8,4(%fp)		# lower 3 bits of fraction

	pushaw	0(%fp)			# push address of rounded result
	movaw	8(%fp),%r2		# pointer to return value
	call	&6,_getfltsw		# raise exception
	movw	0(%r0),%r0		# return the result
	ret	&6
#--------------------------------------------------------------------------
.over_trap:
#					# bias the exponent by -192
#					# for trapped result
#	extzv	&SEXP_OFFSET,&SEXP_WIDTH,%r0,%r1
#	subw2	&192,%r1
#	insv	%r1,&SEXP_OFFSET,&SEXP_WIDTH,%r0

	pushw	&FP_OVFLW		# push exception type
	jmp	.share	
#--------------------------------------------------------------------------
.inf_nan:
	extzv	&0,&20,%r0,%r2		# high fraction bits
	orw2	%r2,%r8			# if ( frac != 0)
	jnz	.nan			#	number is NaN
					# else	number is infinity

	andw2	&SIGN_BIT,%r0		# generate sign bit
	orw2	&SINFINITY,%r0		# put in the right sign
	ret	&6
#--------------------------------------------------------------------------
.nan:
	bitw	&D_NAN_BIT,%r0		# if (NaN bit == 0)
	je	.tnan			#	Trapping NaN
.qnan:					# else	propagate the quiet NaN
	andw2	&SIGN_BIT,%r0		# preserce old sign
	orw2	&SINFINITY,%r0		# put in the reserved exponent
	llsw2	&3,%r8			# top 20 bits of fraction
	orw2	%r8,%r0
	lrsw2	&29,%r1			# 3 more bits of fraction
	orw2	%r1,%r0
	ret	&6
#--------------------------------------------------------------------------
.tnan:
	orw2	&INVLD_STICKY,_asr	# set invalid sticky
	bitw	&INVLD_MASK,_asr	# if (invalid trap == enabled)
	jnz	.invalid_trap		#	raise exception
#-------------------------------------------------------------------------
.gen_qnan:				# else	generate quiet NaN
					# in this release return
					# the generic quiet NaN
	movw	&G_S_QNAN,%r0
	ret	&6
#-------------------------------------------------------------------------
	.data
.invalid_format:
	.byte	FP_D,FP_NULL,FP_NULL,FP_F # for invalid trap
#---------------------------------------
	.text
.invalid_trap:
	pushw	&FP_INVLD		# push exception type
	pushw	&FP_CONV		# push type of operation
	pushaw	.invalid_format		# push format for operand, results
	pushaw	0(%ap)			# push address of operand 1
	pushw	&FP_NULL		# no second operand
	pushw	&FP_NULL		# no rounded result
	movaw	0(%fp),%r2		# pointer to return value
	call	&6,_getfltsw		# raise exception
	movw	0(%r0),%r0		# return the result
	ret	&6

	.def	_fdtos;	.val	.;	.scl	-1;	.endef


#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"sround.s"
.ident	"@(#)libc-m32:fp/sround.s	1.3"

#---------------------------------------------------------------------------
# SINGLE PRECISION ROUNDING
# Input:
#	%r8	fraction
#	%r7	sticky bits
#	%r6	exponent
#	%r5	format string. in case trap has to be generated
#	%r3	type of operation
#	%r0	sign bit aligned in place
# Register usage:
#	%r4	tinyness flag
#---------------------------------------------------------------------------
	.data
	.set	SEXP_OFFSET,23		# offset for single exponent
	.set	SEXP_WIDTH,8		# size of single exponent
	.set	SEXP_BIAS,127		# bias for single exponent
	.set	SMIN_EXP,0		# reserved exponent for zero/denormal
	.set	SMAX_EXP,255		# reserved exponent for NaN and Inf
	.set	SFRAC_MASK,0x7fffff	# this mask get the fraction bits
	.set	SIGN_BIT,0x80000000	# to check the sign bit
	.set	SI_BIT,0x800000		# implied bit of single fraction
	.set	OVERFLOW_BIT,0x1000000	# overflow out of implied bit
	.set	SINFINITY,0x7f800000	# single precision +infinity
	.set	SLARGE,0x7f7fffff	# largest single precision value

	.set	RC_OFFSET,22		# for getting the round control bits
	.set	TO_PLUS,0x1		# rounding mode = towards plus
	.set	TO_MINUS,0x2		# rounding mode = towards minus
	.set	TO_ZERO,0x3		# rounding mode = truncate
#---------------------------------------------------------------------------
	.text
	.align	4
	.globl	_round.single
#---------------------------------------------------------------------------
_round.single:
	addw2	&FP_UNION+8,%sp		# allocate space for return value
					# + rounded result + format string

	movw	&0,%r4			# clear underflow flag

	cmpw	%r6,&SMIN_EXP		# if (exp <= MIN_EXP)
	jle	.denormal		#	do denormal processing
.test_inexact:
	TSTW	%r7			# if ( no sticky)
	je	.pack			#	try to pack
	orw2	&INEXACT_STICKY,_asr	# set inexact sticky
#----------------------------------------
	extzv	&RC_OFFSET,&2,_asr,%r1	# get rounding mode
	jne	.other_modes
.to_near:
	bitw	&2,%r7			# if ( R == 0 )
	je	.pack			#	inexact, no rounding
	bitw	&1,%r8			# if ( L set)
	jne	.round_up		#	round up
	bitw	&1,%r7			# if ( S ==0)
	je	.pack			#	inexact, no rounding
#---------------------------------------------------------------------------
.round_up:
	addw2	&1,%r8			# frac = frac + 1

	TSTW	%r6			# if (exp == 0)
	je	.small			#	check for rounding normal
	bitw	&OVERFLOW_BIT,%r8	# if (rounding overflow)
	jne	.over_round		#	renormalize
#---------------------------------------------------------------------------
	.align	4
.pack:
	insv	%r6,&SEXP_OFFSET,&SEXP_WIDTH,%r0 # put exponent bits
	insv	%r8,&0,&23,%r0		# put in fraction bits

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
############################################################################
.denormal:
	jmp	.denorm
#---------------------------------------------------------------------------
.small:
	bitw	&SI_BIT,%r8		# did the denormal number
					# become normal after ronding
	je	.pack
	addw2	&1,%r6			#	exp ++
	jmp	.pack
#---------------------------------------------------------------------------
.over_round:
	lrsw2	&1,%r8			# frac >>1
	addw2	&1,%r6			# exp ++
	jmp	.pack
#---------------------------------------------------------------------------
.check_inxact_trap:
	bitw	&IM_MASK,_asr
	je	.ret6
					# raise inexact trap
	pushw	&FP_INEXACT		# push exception type
	jmp	.share
#---------------------------------------------------------------------------
	.data

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

	.set	FP_UNION,0xc		# size of return value fp_union

	.set	FP_F,3			# source is single
	.set	FP_NULL,0
	.set	TWO_OP,0x03030000	# format string which says two
					# floating point operands
#--------------------------------------------------------------------------
	.text
.under:
	bitw	&UM_MASK,_asr		# if ( UM == 1 )
	jne	.under_trap		#	raise underflow trap
					# otherwise underflow sticky is
					# to be set if both tiny and inexact
	TSTW	%r7			# if ( ! inexact)
	je	.ret6			#	return
	orw2	&UFLW_STICKY,_asr	# else	set inexact flag
	jmp	.pack2			#	check if to raise trap
#--------------------------------------------------------------------------
.over:
	orw2	&OVFLW_STICKY,_asr
	bitw	&OM_MASK,_asr		# if ( OM == 1 )
	jne	.over_trap		#	raise overflow trap
					# else depends on rounding mode
	orw2	&INEXACT_STICKY,_asr	# if overflow trap is masked
					# set inexact sticky
	extzv	&RC_OFFSET,&2,_asr,%r1	# get rounding mode
	je	.over_near
	cmpw	%r1,&TO_ZERO
	je	.over_zero
	cmpw	%r1,&TO_PLUS
	je	.over_plus
	jmp	.over_minus
#---------------------------------------
.over_near:				# result is signed infinity
	andw2	&SIGN_BIT,%r0
	orw2	&SINFINITY,%r0
	jmp	.check_inxact_trap
#---------------------------------------
.over_zero:				# result is signed largest finite
					# number
	andw2	&SIGN_BIT,%r0
	orw2	&SLARGE,%r0
	jmp	.check_inxact_trap
#---------------------------------------
.over_plus:
	TSTW	%r0			# if positive
	jpos	.over_near		#	+infinity
	jmp	.over_zero		# else	-largest finite
#---------------------------------------
.over_minus:
	TSTW	%r0			# if positive
	jpos	.over_zero		#	+largest finite
	jmp	.over_near		# else	-infinity
#--------------------------------------------------------------------------
#       denormal processing
.denorm:
	orw3	%r7,%r8,%r1		# if ( (mant|sticky) != 0)
	jne	.set_tiny		#	tiny
					# else	return true zero
	extzv	&RC_OFFSET,&2,_asr,%r1	#	get rounding mode
	movw	&0,%r0			#	sign = 0
	cmpw	%r1,&TO_MINUS		#	if (rounding mode!=minus)
	jne	.pack			#		return +0
	movw	&SIGN_BIT,%r0		#	else	return -0
	jmp	.pack

.set_tiny:
	movw	&1,%r4			# remember tinyness
	bitw	&UM_MASK,_asr		# if (underflow trap==enabled)
	je	.no_u_trap
	addw2	&192,%r6		# 	IEEE: for trapped undrflow
					#	multiply inifintely precise
					#	result by 2**192
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
	jmp	.test_inexact
#---------------------------------------- end denormal processing
.other_modes:
	cmpw	%r1,&TO_ZERO
	je	.pack			# rounding mode = truncate
	cmpw	%r1,&TO_PLUS
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
############################################################################
.under_trap:
	orw2	&UFLW_STICKY,_asr	# set underflow sticky

#	The interface routine _getfltsw() is defined as
#	fp_union
#	_getfltsw(ftype, optype, format, op1, op2, rres)
#	fp_ftype	ftype	/* type of fault */
#	fp_op		optype	/* type of operation */
#	char		*format /* format of the operands etc. */
#	fp_union	*op1	/* operand 1 */
#	fp_union	*op2	/* operand 2 */
#	fp_union	*rres	/* rounded result */
#
	pushw	&FP_UFLW		# push exception type

.share:
	movw	%r0,0(%fp)		# store rounded result
	movw	%r5,4(%fp)		# store format string

	pushw	%r3			# push type of operation
	pushaw	4(%fp)			# push format string
	pushaw	0(%ap)			# push address of operand 1

					# rems,sqrs has one operand,
					# others have two.
	andw3	&0xffff0000,%r5,%r1	# get the upper part of format
	cmpw	%r1,&TWO_OP		# if (two operands)
	je	.two_arg		#	push address of the second
	pushw	&FP_NULL		# else	no second operand
	jmp	.rresult
.two_arg:
	pushaw	4(%ap)			# push address of second operand

.rresult:
	pushaw	0(%fp)			# push address of rounded result
	movaw	8(%fp),%r2		# address of return value
	call	&6,_getfltsw		# raise exception
	movw	0(%r0),%r0		# return the result
	ret	&6
#--------------------------------------------------------------------------
.over_trap:
					# bias the exponent by -192
					# for trapped result
	subw2	&192,%r6
	insv	%r6,&SEXP_OFFSET,&SEXP_WIDTH,%r0
	pushw	&FP_OVFLW		# push exception type
	jmp	.share	
#--------------------------------------------------------------------------


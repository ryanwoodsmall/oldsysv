#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"dround.s"
.ident	"@(#)libc-m32:fp/dround.s	1.7"
# The routine "_round.double" rounds double precision floating point numbers.

# This routine checks for:
#	a) Underflow
#	b) Overflow
#	c) Inexact
# and raises appropiate exceptions if required.

# The inputs to the routine are 
#	type of operation 	  = %r3 ( as defined in <ieeefp.h> )
#	sign flag		  = %r4
#	exponent		  = %r5
#	the high part of fraction = %r6
#	the low part of fraction  = %r7
#	RS bits			  = %r8

	.set	L_BIT,1			# least significant bit of fraction
	.set	R_BIT,2			# Round bit
	.set	S_BIT,1			# sticky bit
	.set	I_BIT,0x100000		# implied bit
	.set	V_BIT,0x200000		# overflow out of implied bit

# Register usage:	%r2 is used as a flag to remember tinyness

# Different routines set up these registers and JMPs to this code,
# as opposed to calling this code.
#
# If all goes well, it returns to user code with a double precision return value.
# If an exception is raised, the exception handler returns control to this routine,
# which then returns to user code.

	.set	MAX_EXP,0x7ff		# reserved value for NaN & Infinity
	.set	MIN_EXP,0		# reserved value for zero & denormal
	.set	TO_NEAREST,0x0		# round to nearest mode
	.set	TO_PLUS,0x1		# round to plus
	.set	TO_MINUS,0x2		# round to minus
	.set	TO_ZERO,0x3		# round to zero, i.e. truncate.
	.set	RC_OFFSET,22		# offset for round control bits in asr

	.set	UFLW_STICKY,0x80	# to set underflow sticky
	.set	UM_MASK,0x1000		# offset for underflow mask
	.set	FP_UFLW,4		# exception type = underflow

	.set	INEXACT_STICKY,0x40000	# for setting inexact sticky bit
	.set	IM_MASK,0x400		# to check inexact mask bit
	.set	FP_INEXACT,5		# exception type = inexact

	.set	OVFLW_STICKY,0x100	# for setting overflow mask
	.set	OM_MASK,0x2000		# to check overflow mask
	.set	FP_OVFLW,3		# exception type = overflow

	.set	FP_UNION,0xc		# size of return value fp_union
	.set	FP_D,4			# type of operands,fault value and result
	.data
	.align	4
.format:
	.byte	FP_D,FP_D,FP_D,FP_D
#-------------------------------------------------------------------------------
#	These pieces of code are brought before the entry
#	point in this routine to minimize branches in the
#	code which is expected to be more frequent.

	.text
.check_underflow:
	orw3	%r6,%r7,%r0		# OR of all fraction bits
	orw2	%r8,%r0
	jne	.set_tiny
					# if (frac == 0) return true zero
	extzv	&RC_OFFSET,&2,_asr,%r0	# The result is -0 in RM mode.
	cmpw	%r0,&TO_MINUS
	jne	.plus_zero
	movw	&1,%r4
	jmp	.pack_double		# set sign
.plus_zero:
	movw	&0,%r4			# clear sign
	jmp	.pack_double

.set_tiny:				# if non zero, we have underflow
	MCOMW	%r2,%r2			# set tiny flag
	bitw	&UM_MASK,_asr		# if (underflow mask)
	je	.denorm_loop		#
	addw2	&1536,%r5		#	adjust exponent for trap
	cmpw	%r5,&MIN_EXP		#	check if still denormal
	jg	.round

# adjust for denrormal by shifting right
.denorm_loop:				#   do
	andw3	&1,%r8,%r0		#	save sticky
	lrsw2	&1,%r8			#	new sticky = 
	orw2	%r0,%r8			#	R | S bits
	andw3	&1,%r7,%r0		#	new R =
	llsw2	&1,%r0			#	last bit of result
	orw2	%r0,%r8			#	ORed into R

	lrsw2	&1,%r7			#	frac low >>= 1
	andw3	&1,%r6,%r0		#	lowest bit of frac high
	llsw2	&31,%r0			#	align at the highest bit
	orw2	%r0,%r7			#	and ORed into place
	lrsw2	&1,%r6			#	frac high >>=1
	addw2	&1,%r5			#	exponent += 1

	cmpw	%r5,&MIN_EXP+1		# while ( exp < 1 )
	jl	.denorm_loop
	movw	&0,%r5			# exp = 0
	jmp	.round
#-------------------------------------------------------------------------------
.small:
	jmp	.check_underflow
#-------------------------------------------------------------------------------
.other_modes:
	cmpw	%r0,&TO_ZERO
	je	.to_zero
	cmpw	%r0,&TO_PLUS
	je	.to_plus
	jmp	.to_minus
#-------------------------------------------------------------------------------
	.align	4
.R_clear:
	bitw	%r8,&S_BIT		# is the S bit set?
	jnz	.inexact

.exact:
	cmpw	%r5,&MAX_EXP		# if (overflow)
	jl	.pack_double		# else return result
	jmp	.overflow		#      go to .overflow
#-------- ENTRY POINT OF THE CODE --------------------------------------------
#	.align	4
	.globl	_round.double
_round.double:
	MCOUNT
	CLRW	%r2			# r2 is used as tinyness flag
	cmpw	%r5,&MIN_EXP		# if (exp <= 0)
	jle	.small			# 	check for underflow
#----------------------------------------------------------------------------
.round:
	extzv	&RC_OFFSET,&2,_asr,%r0	# get  rounding mode
	jnz	.other_modes		# if not to-nearest check other modes
#-------------------------------------------------------------------------------
.to_nearest:				# round to nearest is default mode
	bitw	%r8,&R_BIT		# is Round bit set?
	jz	.R_clear
	bitw	%r8,&S_BIT		# is the Sticky bit set?
	jnz	.round_up
	bitw	%r7,&L_BIT		# is the L bit set?
	jz	.inexact

.round_up:				# we have R(L+S) == 1
	INCW	{uword}%r7		# mant_low ++
	BVCB	.no_carry
	TSTW	%r5
	jnz	.carry
#---------------------------------------
# some time a denormal number may become normal after rounding
	andw3	&I_BIT,%r6,%r0		# remember I_BIT
	INCW	%r6			# propagate carry: mant_high++
	andw3	&I_BIT,%r6,%r1
	xorw2	%r0,%r1			# did the I_BIT change?
	je	.inexact		# no
	INCW	%r5			# yes. exp++
	jmp	.inexact
#---------------------------------------
#	.align	4
.carry:
	INCW	%r6
.no_carry:
	bitw	%r6,&V_BIT		# check for 1 bit rounding overflow
	jz	.inexact
					# we have 1 bit rounding overflow
	andw3	&1,%r6,%r0		# save the bit to be shifted out
	lrsw2	&1,%r6			# mant_high >>= 1
	lrsw2	&1,%r7			# mant_low >>= 1
	insv	%r0,&31,&1,%r7		# put in the bit shifted out of mant_high 
	INCW	%r5			# exponent++
#-----------------------------------------
#	.align	4
.inexact:
	orw2	&INEXACT_STICKY,_asr	# set inexact sticky
	cmpw	%r5,&MAX_EXP		# if (exp >= MAX_EXP)
	jge	.overflow		# 	overflow
#--------------------------------------------------------------------------
.check_inexact:
	TSTW	%r2			# if (tinyness)
	jz	.check_inexact_trap	#
	orw2	&UFLW_STICKY,_asr	#	set underflow sticky
	bitw	&UM_MASK,_asr		#	if (underflow enabled)
	jne	.pack_double		#		don't raise inexact

.check_inexact_trap:
	bitw	&IM_MASK,_asr		# is inexact trap enabled ?
	jnz	.inexact_trap		# 	raise inexact exception
#---------------------------------------------------------------------------
#	%r4 = sign
#	%r5 = exponent 
#	%r6 = mantissa high
#	%r7 = mantissa low
# return to user code with packed double-precision value in %r0,%r1

.pack_double:
	llsw3	&20,%r5,%r0		# insert the exponent and
					# clear the sign
	TSTW	%r4
	je	.pack_plus		# if ( result_sign)
	insv	&1,&31,&1,%r0		# sign =  1
.pack_plus:
	insv	%r6,&0,&20,%r0		# insert unpacked mantissa high
	movw	%r7,%r1			# and move the mantissa low
	TSTW	%r2			# if tiny
	jne	.check_under_trap	#	check if to raise trap
.ret6:
	ret	&6
#--------------------------------------------------------------------------
.check_under_trap:
	bitw	&UM_MASK,_asr		# if (underflow mask)
	je	.ret6
					#	raise underflow trap
	orw2	&UFLW_STICKY,_asr	# set underflow sticky
	movw	%sp,%r2
	addw2	&FP_UNION+16,%sp	# allocate for rounded result
	pushw	&FP_UFLW		# push type of exception
	jmp	.exception
#--------------------------------------------------------------------------
.inexact_trap:				# We have inexact exception

	movaw	0(%sp),%r2		# pointer to rounded result
	addw2	&FP_UNION+16,%sp	# allocate for rounded result
					# and the return value
	pushw	&FP_INEXACT		# push type of exception
.exception:
	pushw	%r3			# push type of operation
	llsw3	&20,%r5,%r0		# insert the exponent and
					# clear the sign
	TSTW	%r4
	je	.pack_plus2		# if ( result_sign)
	insv	&1,&31,&1,%r0		# sign =  1
.pack_plus2:
	insv	%r6,&0,&20,%r0		# insert unpacked mantissa high
	movw	%r0,0(%r2)		# put the inexact result in memory
	movw	%r7,4(%r2)		# put the inexact result in memory

	pushaw	.format			# push format for src etc.
	pushaw	0(%ap)			# push pointer to operand 1
	pushaw	8(%ap)			# push pointer to operand 2
	pushw	%r2			# push pointer to rounded result
	addw2	&16,%r2			# pointer to return value
	call	&6,_getfltsw		# raise exception

	movw	4(%r0),%r1		# pick up result
	movw	0(%r0),%r0
	ret	&6			# return to user code.
#--------------------------------------------------------------------------
.overflow:
	orw2	&OVFLW_STICKY,_asr	# set overflow sticky
	bitw	&OM_MASK,_asr		# if (!overflow mask)
	je	.overflow_masked
					# else raise exception
	subw2	&1536,%r5		# bias trapped exponent
	movaw	0(%sp),%r2		# pointer to rounded result
	addw2	&FP_UNION+16,%sp
	pushw	&FP_OVFLW		# push type of exception
	jmp	.exception
#--------------------------------------------------------------------
# The result when overflow is masked depends upon rounding mode.
.overflow_masked:
	orw2	&INEXACT_STICKY,_asr	# set inexact sticky
	extzv	&RC_OFFSET,&2,_asr,%r0	# get rounding mode
	jz	.overflow_near
	cmpw	%r0,&TO_ZERO
	je	.overflow_zero
	cmpw	%r0,&TO_PLUS
	je	.overflow_plus
	jmp	.overflow_minus

.overflow_near:				# in this rounding mode, result is
	movw	&MAX_EXP,%r5		# correctly signed infinity
	CLRW	%r6
	CLRW	%r7
	jmp	.check_inexact

.overflow_zero:				# in this rounding mode, the result
	movw	&MAX_EXP-1,%r5		# is the largest finite
	movw	&0x1fffff,%r6		# number with the right sign
	movw	&0xffffffff,%r7
	jmp	.check_inexact

.overflow_plus:			# in this rounding mode,
	TSTW	%r4
	je	.overflow_near	# for positive overflow, the result
				# is plus infinity. We can share the code.
	jmp	.overflow_zero	# for negative overflow, the result
				# is maximum negative number
				# we can share that code.

.overflow_minus:		# in this rounding mode
	TSTW	%r4
	je	.overflow_zero	# positive overflow becomes largest
				# finite number
	jmp	.overflow_near	# negative overflow becomes
				# minus infinity
#--------------------------------------------------------------------------
.to_zero:	# rounding mode is towards zero
	TSTW	%r8			# if ((round|sticky) != 0)
	jnz	.inexact		# then	inexact result
	jmp	.exact			# else	exact result
#-------------------------------------------------------------------------
.to_plus:	# rounding mode is towards plus
	TSTW	%r8			# if ( (round|sticky) == 0 )
	jz	.exact			# then	exact result

	TSTW	%r4			# else	if (sign == positive)
	jz	.round_up		#		round up
	jmp	.inexact		# else	inexact result
#--------------------------------------------------------------------------
.to_minus:	# rounding mode is towards minus

	TSTW	%r8			# if ( (round & sticky) == 0 )
	jz	.exact			# then	exact result

	TSTW	%r4			# else	if (sign == negative)
	jne	.round_up		#	we have to round up
					#	(the magnitude)
	jmp	.inexact		# else	inexact
#--------------------------------------------------------------------------

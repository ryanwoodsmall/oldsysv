#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fltos.s"
.ident	"@(#)libc-m32:fp/fltos.s	1.7"
#	convert long to single precision floating.
#	float	_fltos(srcL)
#	long	srcL;

#	Format of packed singles:
#
#	  31  30     23  22			      0
#	+---+----------+-------------------------------+
#	| S | exponent |	mantissa = 23 bits     |
#	+---+----------+-------------------------------+
#
	.data
	.set	EXP_OFFSET,23		# for inserting the exponents
	.set	EXP_WIDTH,8		#	" "
	.set	MANT_WIDTH,23		# for inserting the mantissa
	.set	IMPLIEDBIT,0x800000

#	Register usage:
#	%r1	mantissa
#	%r2	exponent
#	%r7	sign
#	%r8	for caching constants
#	%r0	scratch
#-----------------------------------------------------------------------------
	.text
	.align	4
	.def	_fltos;	.val	_fltos;	.scl	2;	.type	046;	.endef
	.globl	_fltos
#-----------------------------------------------------------------------------
_fltos:
	MCOUNT
	TSTW	_fp_hw
	je	.software
#----------------------------------
.mau:
	mmovws	0(%ap),0(%ap)
	movw	0(%ap),%r0
	RET
#----------------------------------------------------------------------------
.zero:					# return +0.0 which is same as 0
	movw	%r1,%r0			# we know that %r1 is zero
	RET
#----------------------------------------------------------------------------
.max_neg:
	movw	&0xcf000000,%r0		# This is the float equivalent
					# of -2**31
	POPW	%r8
	POPW	%r7
	RET
#----------------------------------------------------------------------------
.software:
	movw	0(%ap),%r1
	je	.zero			# if ( arg == 0 ) return zero
	pushw	%r7			# save %r7
	pushw	%r8			# save %r8
	movw	%r1,%r7			# remember the sign
	jpos	.unsigned		# else	if (arg is positive) skip
	mnegw	%r1,%r1			# else	make it positive.
	BVSB	.max_neg		# |max negative| number is bigger
					# than maximum positive number
.unsigned:
	movw	&150,%r2		# exp = EXPBIAS + FRACSIZE = 127 + 23
	movw	&IMPLIEDBIT,%r8
	llsw3	&1,%r8,%r0		# %r0 = 2 * IMPLIEDBIT
	cmpw	%r1,%r0			# if ( frac > ( 2*IMPLIEDBIT - 1 ))
	jgeu	.large
#-----------------------------------------------------------------------------
.small:
	cmpw	%r1,&4096		# if ( frac < (1<<12) )
	jgeu	.small_2
	llsw2	&12,%r1			#	frac <<= 12
	subw2	&12,%r2			#	exp   -= 12
.small_2:
	cmpw	%r1,&262144		# if ( frac < (1<<18) )
	jgeu	.lshift_end
	llsw2	&6,%r1			#	frac <<= 6
	subw2	&6,%r2			#	exp   -= 6
	jmp	.lshift_end

.lshift:				# do
	llsw2	&1,%r1			# frac <<= 1
	subw2	&1,%r2			# exp  -=  1
.lshift_end:
	bitw	%r1,%r8			# while ( frac & IMPLIEDBIT != 0)
	jz	.lshift
#---------------------------------------------------------------------------
.pack:					# pack the result in float format
	andw3	&0x80000000,%r7,%r0	# get the sign of the result
	orw2	%r1,%r0			# OR in the frac
	insv	%r2,&EXP_OFFSET,&EXP_WIDTH,%r0# insert the exponent
	POPW	%r8			# restore %r8
	POPW	%r7
	RET
#-------------------------------------------------------------------------
.large:
#	Before rounding, the format for fraction part used is:
#
#	  31  30  26  25  24		     2   1   0	bit positions
#	+--------+---+---+-----------------+---+---+---+
#	| unused | V | I |   bits 23 to 1  | L | R | S |
#	+--------+---+---+-----------------+---+---+---+
#		   ^   ^		     ^   ^   ^
#	  	   |   |		     |   |   |
#	  	   |   |		     |   |   +-	sticky bit
#	  	   |   |		     |   +-----	round bit
#	  	   |   |		     +--------- least significant bit
#	  	   |   |
#	  	   |   +-------------------------------	implicit 1 bit
#	  	   +-----------------------------------	place for overflow
#
	.set	IMPLICIT_1,0x2000000	# (before rounding)
	.set	MASK,0xfc000000		# to check bits 26-31
	.set	OVERFLOW_BIT,0x1000000	# (after rounding and alignment)
	.set	RS_BITS,3		# to check for RS bits
	addw2	&2,%r2			# adjust exponent for RS bits
	movw	&MASK,%r8
	movw	&0,%r0			# clear %r0
	jmp	.rshift_end

.rshift:				# do
	orw2	%r1,%r0			# 	save sticky
	lrsw2	&1,%r1			# 	frac >>=1
	addw2	&1,%r2			# 	exp ++
.rshift_end:
	bitw	%r1,%r8			# while ( frac & mask != 0 )
	jnz	.rshift

	andw2	&1,%r0			# extract sticky
	orw2	%r0,%r1			# OR in sticky

	bitw	%r1,&IMPLICIT_1		# one bit left shift is needed
	jnz	.round
	llsw2	&1,%r1			# for numbers which are in the
	subw2	&1,%r2			# range (1<<24) to (1<<25 -1)
#-------------------------------------------------------------------------
	.set	TO_NEAREST,0x0
	.set	TO_PLUS,0x1
	.set	TO_MINUS,0x2
	.set	TO_ZERO,0x3
	.set	RC_OFFSET,22
.round:
	extzv	&RC_OFFSET,&2,_asr,%r0
	jz	.to_nearest
	cmpw	%r0,&TO_ZERO
	je	.to_zero
	cmpw	%r0,&TO_PLUS
	je	.to_plus
#-------------------------------
.to_minus:
	bitw	%r1,&RS_BITS		# is RS == 0 ?
	jz	.exact
	TSTW	0(%ap)			# was the operand positive ?
	jpos	.LS_clear
					# we have to round down
	jmp	.round_up		# i.e. round up the magnitude
#-------------------------------
.to_plus:
	bitw	%r1,&RS_BITS		# is RS == 0 ?
	jz	.exact
	TSTW	0(%ap)			# was the operand positive?
	jneg	.LS_clear
	jmp	.round_up
#-------------------------------
.to_zero:
	bitw	%r1,&RS_BITS
	jz	.exact
	jmp	.LS_clear
#-------------------------------
.to_nearest:				# round to nearest is default mode
	bitw	%r1,&2			# is R bit set?
	jz	.R_clear
	bitw	%r1,&5			# is the L or the S bit set?
	jz	.LS_clear
		
.round_up:				# we have R(L+S) == 1
	lrsw2	&2,%r1
	addw2	&1,%r1			# frac = (frac >> 2) + 1 
	bitw	%r1,&OVERFLOW_BIT
	jz	.inexact
					# we have 1 bit rounding overflow
	lrsw2	&1,%r1			# mantissa >>= 1
	addw2	&1,%r2			# exponent ++

#------------------------------------
	.set	IS,0x40000		# inexact sticky bit
	.set	IM_MASK,0x400		# to check inexact mask bit
	.set	FP_INEXACT,5		# fault type
	.set	FP_CONV,6		# operation type
	.set	FP_L,2			# type of operand
	.set	FP_F,3			# type of result
	.set	FP_NULL,0
	.set	FP_UNION,0xc		# size  of return value
	.data
	.align	4
.formats:
	.byte	FP_L,FP_NULL,FP_F,FP_F
#--------------------------------------------------------------------
	.text
.inexact:
	orw2	&IS,_asr		# set inexact sticky
	bitw	&IM_MASK,_asr		# is inexact trap enabled ?
	je	.pack			# no, pack and return

	andw3	&0x80000000,%r7,%r0	# get the sign of the result
	orw2	%r1,%r0			# OR in the frac
	insv	%r2,&EXP_OFFSET,&EXP_WIDTH,%r0# insert the exponent
	POPW	%r8			# restore register %r8
	POPW	%r7			# restore register %r7

	save	&0			# we have to allocate stack space
					# to pass the result to handler
	addw2	&FP_UNION+4,%sp		# allocate space
	movw	%r0,0(%fp)		# store the rounded result

	pushw	&FP_INEXACT		# push type of fault
	pushw	&FP_CONV		# push type of operation
	pushaw	.formats		# push format for src and dst
	pushaw	0(%ap)			# push address of operand 1
	pushw	&FP_NULL		# no second operand, push null
	pushaw	0(%fp)			# push address of rounded result
	movaw	4(%fp),%r2		# pointer to return value
	call	&6,_getfltsw		# raise exception
	movw	0(%r0),%r0		# pick up result
	ret	&0			# return to user code.
#------------------------------------
.R_clear:
	bitw	%r1,&1			# is the S bit set?
	jz	.exact
.LS_clear:
	lrsw2	&2,%r1			# frac >>= 2
	jmp	.inexact

.exact:
	lrsw2	&2,%r1			# frac >>= 2
	jmp	.pack
#------------------------------------------------------------------------
	.def	_fltos;	.val	.;	.scl	-1;	.endef
#########################################################################
#	float	futos(srcU)
#	unsigned srcU		/* unsigned to float conversion */
	.text
	.align	4
	.def	_futos;	.val	_futos;	.scl	2;	.type	046;	.endef
	.globl	_futos
_futos:
	MCOUNT
	TSTW	_fp_hw
	je	.usoft
#-----------------------
	mmovus	0(%ap),0(%ap)
	movw	0(%ap),%r0
	RET
#-----------------------
.usoft:
	movw	0(%ap),%r1
	je	.zero
	pushw	%r7
	pushw	%r8
	movw	&0,%r7			# sign = positive
	jmp	.unsigned

	.def	_futos;	.val	.;	.scl	-1;	.endef

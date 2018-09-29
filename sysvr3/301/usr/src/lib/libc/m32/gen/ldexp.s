#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:gen/ldexp.s	1.9"
	.file	"ldexp.s"
#	double ldexp (value, exp)
#		double value;
#		int exp;
#
#	Ldexp returns value * 2**exp, if that result is in range.
#	If underflow occurs, it returns zero.  If overflow occurs,
#	it returns a value of appropriate sign and largest possible single-
#	precision magnitude.  In case of underflow or overflow,
#	the external int "errno" is set to ERANGE. If value is
#	NAN or infinity errno is set to EDOM. Note that errno is
#	not modified if no error occurs, so if you intend to test it
#	after you use ldexp, you had better set it to something
#	other than ERANGE first (zero is a reasonable value to use).
#-----------------------------------------------------------------------------
	.data
	.align	4
.huge:
	.word	0x47efffff,0xe0000000	# The largest number that can
				#   be represented in a short floating
				#   number.  This is given in hex in order
				#   to avoid floating conversions.
.2TO52:
	.word	0x43300000,0

	.set	DEXP_OFFSET,20	# bit offset where the exponent starts
	.set	DEXP_WIDTH,11	# width of double exponent
	.set	MAX_EXP,0x7ff	# maximum exponent of double number
	.set	SIGN_MASK,0x7fffffff	# used to mask off sign
	.set	SIGN_BIT,0x80000000
	.set	IMPLIED_BIT,0x800000	# used to check normalization
	.set	ERANGE,34	# error number for range error
	.set	EDOM,33		# domain error
	.globl	errno		# error flag
#-------------------------------------------------------------------------
	.text
	.align	4
	.def	ldexp;	.val	ldexp;	.scl	2;	.type	047;	.endef
	.globl	ldexp
#-------------------------------------------------------------------------
ldexp:
	save	&1		# Uses r8
	MCOUNT
	movw	0(%ap),%r0	# Fetch sign+exponent+high part of mantissa
	movw	4(%ap),%r1	# Fetch low part of mantissa
	andw3	&SIGN_MASK,%r0,%r8 # bits other than sign
	orw2	%r1,%r8		# if zero, we are done
	jz	.done		# If it's zero, we're done

	movw	8(%ap),%r8	# r8 := amount to offset exponent
	jz	.done		# If it's zero, we're done

	EXTFW	&DEXP_WIDTH-1,&DEXP_OFFSET,%r0,%r2	# biased exponent
	jz	.denormal	# If exponent is 0, input is denormalized
	cmpw	%r2,&MAX_EXP
	je	.nan_inf
#-------------------------------------------------------------------------
.norm:
	addw2	%r8,%r2		# r2 := new biased exponent
	BVSB	.over		# Check that the addition didn't overflow
	jnpos	.under		# If it's <= 0, we have an underflow
	cmpw	%r2,&MAX_EXP	# Otherwise check if it's too big
	jge	.over		# if ( exp >= MAX_EXP) then overflow
#-------------------------------------------------------------------------
#	Construct the result by jamming the exponent and return

	insv	%r2,&DEXP_OFFSET,&DEXP_WIDTH,%r0	# Put the exponent
							# back in the result
.done:
	ret	&1
#-------------------------------------------------------------------------
.nan_inf:			# Returns the input number
	movw	&EDOM,errno	# indicate domain error
	ret	&1
#-------------------------------------------------------------------------
.denormal:			# Multiply by 2 ** 52
	pushw	.2TO52		# to guarantee a normalized number
	pushw	.2TO52+4
	pushw	%r0
	pushw	%r1
	call	&4,_fmuld
	EXTFW	&DEXP_WIDTH-1,&DEXP_OFFSET,%r0,%r2	# biased exponent
	subw2	&52,%r2		# Reduce biased exponent
	jmp	.norm
#-------------------------------------------------------------------------
#	Underflow
.under:
	cmpw	%r2,&-52	# If it's too small for denormalized
	jl	.zero		#    result, return zero
	insv	&1,&20,&11,%r0	# Reset the exponent in the result
	addw2	&1022,%r2	# Prepare exponent for negative power of 2
	llsw2	&20,%r2		# Shift exponent into position
	pushw	%r0
	pushw	%r1
	pushw	%r2
	pushw	&0
	call	&4,_fmuld	# Multiply to produce denormalized result
	jmp	.done
#-------------------------------------------------------------------------
#	Return zero result
.zero:
	movw	&0,%r0		# Result is zero
	movw	%r0,%r1
	jmp	.err		# Join general error code
#-------------------------------------------------------------------------
#	Overflow
.over:
	movw	.huge,%r0	# Largest possible floating magnitude
	movw	.huge+4,%r1
	jbc	&31,0(%ap),.err	# Jump if argument was positive
	orw2	&SIGN_BIT,%r0	# If arg < 0, make result negative
#-------------------------------------------------------------------------
.err:
	movw	&ERANGE,errno	# Indicate range error
	jmp	.done
#------------------------------------------------------------------------
	.def	ldexp;	.val	.;	.scl	-1;	.endef

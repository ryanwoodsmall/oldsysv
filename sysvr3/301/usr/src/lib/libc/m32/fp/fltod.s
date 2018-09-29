#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fltod.s"
.ident	"@(#)libc-m32:fp/fltod.s	1.5"
#	convert signed long integer to double precision floating.
#	double	fltod(srcL)
#	long	srcL;

	.data
	.set	EXP_OFFSET,20		# for inserting the exponent
	.set	EXP_WIDTH,11		# size of double exponent
	.set	EXP_BIAS,1023		# bias for double exponent

#	Register usage:
#	%r0	scratch
#	%r1	integer
#	%r2	exponent
#-----------------------------------------------------------------------------
	.text
	.globl	_fp_hw
	.globl	_asr
	.globl	_getfltsw
#-----------------------------------------------------------------------------
	.align	4
	.def	_fltod;	.val	_fltod;	.scl	2;	.type	047;	.endef
	.globl	_fltod
#-----------------------------------------------------------------------------
_fltod:
	MCOUNT
	TSTW	_fp_hw
	je	.software
#----------------------------------
.mau:
	save	&0
	addw2	&8,%sp
	mmovwd	0(%ap),0(%fp)
	movw	4(%fp),%r1
	movw	0(%fp),%r0
	ret	&0
#----------------------------------------------------------------------------
.zero:					# return +0.0 which is same as 0
	movw	%r1,%r0			# we know that %r1 is zero
	RET
#----------------------------------------------------------------------------
.max_neg:
	movw	&0xc1e00000,%r0		# This is the double equivalent
	movw	&0,%r1			# of -2**31
	RET
#----------------------------------------------------------------------------
.software:
	movw	0(%ap),%r1
	je	.zero			# if ( arg == 0 ) return zero
	jpos	.unsigned		# else	if (arg is positive) skip
	mnegw	%r1,%r1			# else	make it positive.
	BVSB	.max_neg		# |max negative| number is bigger
					# than maximum positive number

.unsigned:		# align the integer so that the 31st bit is 1
			# we do a 3 level unrolled binary search.

	movw	&EXP_BIAS+31,%r2	# exponent = BIAS + 31
	movw	&0x80000000,%r0		# mask, has only 31st bit on

	cmpw	%r1,&0xffff
	jgu	.bigger1
	llsw2	&16,%r1
	subw2	&16,%r2
.bigger1:
	cmpw	%r1,&0xffffff
	jgu	.bigger2
	llsw2	&8,%r1	
	subw2	&8,%r2
.bigger2:
	cmpw	%r1,&0xfffffff
	jgu	.lshiftend
	llsw2	&4,%r1
	subw2	&4,%r2
	jmp	.lshiftend

.lshift:
	llsw2	&1,%r1
	subw2	&1,%r2
.lshiftend:
	bitw	%r0,%r1
	jz	.lshift
#---------------------------------------------------------------------------
.pack:					# pack the result in double format
	andw2	0(%ap),%r0		# get the sign of argument
	llsw2	&20,%r2			# align exponent
	orw2	%r2,%r0			# insert exponent
	extzv	&11,&21,%r1,%r2		# extract the top 21 bits
	insv	%r2,&0,&20,%r0
	llsw2	&21,%r1
	RET
#------------------------------------------------------------------------------
	.def	_fltod;	.val	.;	.scl	-1;	.endef
###############################################################################
#	convert unsigned long to double precision floating.
#	double	futod(srcU)
#	unsigned srcU;
	.text
	.align	4
	.def	_futod;	.val	_futod;	.scl	2;	.type	047;	.endef
	.globl	_futod
#-----------------------------------------------------------------------------
_futod:
	MCOUNT
	TSTW	_fp_hw
	je	.usoft
#----------------------------------
	save	&0
	addw2	&8,%sp
	mmovud	0(%ap),0(%fp)
	movw	4(%fp),%r1
	movw	0(%fp),%r0
	ret	&0
#-----------------------------------------------------------------------------
.usoft:
	movw	0(%ap),%r1
	je	.zero			# if ( arg == 0 ) return zero
	movw	&0,0(%ap)		# make sure that the sign looks
					# positive
	jmp	.unsigned		# else	treat it as positive

	.def	_futod;	.val	.;	.scl	-1;	.endef

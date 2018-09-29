#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"isnand.s"
.ident	"@(#)libc-m32:fp/isnand.s	1.1"
#	int isnand(srcD)
#	double srcD;
#
#	This routine returns 1 if the argument is a NaN
#		     returns 0 otherwise.

	.set	DMAX_EXP,0x7ff

	.text
	.align	4
	.def	isnand;	.val	isnand;	.scl	2;	.type	047;	.endef
	.globl	isnand
isnand:
	MCOUNT
	extzv	&20,&11,0(%ap),%r0
	cmpw	%r0,&DMAX_EXP		# if ( exp != 0x7ff )
	jne	.false			#	its not a NaN
	extzv	&0,&20,0(%ap),%r0
	orw2	4(%ap),%r0		# OR of all the fraction bits
	je	.false			# if ( fraction == 0 )
					#	its an infinity
	movw	&1,%r0
	RET
.false:
	movw	&0,%r0
	RET
	.def	isnand;	.val	.;	.scl	-1;	.endef


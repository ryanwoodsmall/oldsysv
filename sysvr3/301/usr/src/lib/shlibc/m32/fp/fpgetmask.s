#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fpgetmask.s"
.ident	"@(#)libc-m32:fp/fpgetmask.s	1.6"
#----------------------------------
	.data
	.set	IM,14		# invalid mask
	.set	OM,13		# overflow mask
	.set	UM,12		# underflow mask
	.set	QM,11		# divide by zero mask
	.set	PM,10		# inexact mask
#----------------------------------
	.text
	.globl	_fp_hw		# to check hardware presence
	.globl	_asr		# simulated arithmetic status register
	.globl	_fpstart	# to force floating point startup
#----------------------------------
	.align	4
	.def	fpgetmask;	.val	fpgetmask;	.scl	2;	.type	052;	.endef
	.globl	fpgetmask
fpgetmask:
	MCOUNT
	TSTW	_fp_hw
	je	.soft
#----------------------------------
# MAU
	save	&0
	addw2	&4,%sp
	mmovfa	0(%fp)
	extzv	&PM,&5,0(%fp),%r0	# old mask bits
	ret	&0
#----------------------------------
	.align	4
.soft:					# running software
	extzv	&PM,&5,_asr,%r0		# old mask bits
	RET

	.def	fpgetmask;	.val	.;	.scl	-1;	.endef

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fpgetsticky.s"
.ident	"@(#)libc-m32:fp/fpgetsticky.s	1.6"
#----------------------------------
	.data
	.set	PS,18		# inexact sticky
	.set	IS,9		# invalid sticky
	.set	OS,8		# overflow sticky
	.set	US,7		# underflow sticky
	.set	QS,6		# divide by zero sticky
#----------------------------------
	.text
	.globl	_fp_hw		# to check hardware presence
	.globl	_asr		# simulated arithmetic status register
	.globl	_fpstart	# to force floating point startup
#----------------------------------
	.align	4
	.def	fpgetsticky;	.val	fpgetsticky;	.scl	2;	.type	052;	.endef
	.globl	fpgetsticky
fpgetsticky:
	MCOUNT
	TSTW	_fp_hw
	je	.soft
#----------------------------------
# MAU
	save	&0
	addw2	&4,%sp
	mmovfa	0(%fp)
	movw	0(%fp),%r2
	extzv	&QS,&4,%r2,%r0		# get IS,OS,US,QS
	llsw2	&1,%r0			# << 1 for inexact sticky
	extzv	&PS,&1,%r2,%r1		# get PS
	addw2	%r1,%r0			# add to the others
	ret	&0
#----------------------------------
	.align	4
.soft:					# running software
	movw	_asr,%r2
	extzv	&QS,&4,%r2,%r0		# get IS,OS,US,QS
	llsw2	&1,%r0			# << 1 for inexact sticky
	extzv	&PS,&1,%r2,%r1		# get PS
	addw2	%r1,%r0			# add to the others
	RET

	.def	fpgetsticky;	.val	.;	.scl	-1;	.endef

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fpsetmask.s"
.ident	"@(#)libc-m32:fp/fpsetmask.s	1.4.1.2"
#----------------------------------
	.data
	.set	IM,14		# invalid mask
	.set	OM,13		# overflow mask
	.set	UM,12		# underflow mask
	.set	QM,11		# divide by zero mask
	.set	PM,10		# inexact mask
	.set	CSC_BIT,0x20000	# context switch control bit in ASR
				# is manually set after a "mmovta"
				# so that the new ASR is not lost on switch
#----------------------------------
	.text
	.globl	_fp_hw		# to check hardware presence
	.globl	_asr		# simulated arithmetic status register
#----------------------------------
	.align	4
	.def	fpsetmask;	.val	fpsetmask;	.scl	2;	.type	052;	.endef
	.globl	fpsetmask
#----------------------------------
fpsetmask:
	MCOUNT
	TSTW	_fp_hw
	je	.soft
#----------------------------------
# MAU
	save	&0
	addw2	&4,%sp
	mmovfa	0(%fp)			# read ASR
	extzv	&PM,&5,0(%fp),%r0	# old mask bits
	insv	0(%ap),&PM,&5,0(%fp)	# put in new mask bits
	orw2	&CSC_BIT,0(%fp)		# make sure this ASR is saved
	mmovta	0(%fp)			# on a context switch
	ret	&0
#----------------------------------
	.align	4
.soft:					# running software
	extzv	&PM,&5,_asr,%r0		# old mask bits
	insv	0(%ap),&PM,&5,_asr	# put in new mask bits
	RET

	.def	fpsetmask;	.val	.;	.scl	-1;	.endef

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fpsetsticky.s"
.ident	"@(#)libc-m32:fp/fpsetsticky.s	1.4.1.2"
#----------------------------------
	.data
	.set	PS,18		# inexact sticky
	.set	IS,9		# invalid sticky
	.set	OS,8		# overflow sticky
	.set	US,7		# underflow sticky
	.set	QS,6		# divide by zero sticky
	.set	CSC_BIT,0x20000	# context switch control bit in ASR
				# is manually set after a "mmovta"
				# so that the new ASR is not lost on switch
#----------------------------------
	.text
	.globl	_fp_hw		# to check hardware presence
	.globl	_asr		# simulated arithmetic status register
#----------------------------------
	.align	4
	.def	fpsetsticky;	.val	fpsetsticky;	.scl	2;	.type	052;	.endef
	.globl	fpsetsticky
#----------------------------------
fpsetsticky:
	MCOUNT
	TSTW	_fp_hw
	je	.soft
#----------------------------------
# MAU hardware running
	save	&0
	addw2	&4,%sp
	mmovfa	0(%fp)
	movw	0(%fp),%r2
	extzv	&QS,&4,%r2,%r0		# get IS,OS,US,QS
	llsw2	&1,%r0			# << 1 for inexact sticky
	extzv	&PS,&1,%r2,%r1		# get PS
	addw2	%r1,%r0			# add to the others

	lrsw3	&1,0(%ap),%r1		# align IS,OS,US,QS
	insv	%r1,&QS,&4,%r2		# put in IS,OS,US,QS
	insv	0(%ap),&PS,&1,%r2	# put in PS
	movw	%r2,0(%fp)		# write in memory
	orw2	&CSC_BIT,0(%fp)		# make sure this ASR is saved
	mmovta	0(%fp)
	ret	&0
#----------------------------------
	.align	4
.soft:					# running software
	movw	_asr,%r2
	extzv	&QS,&4,%r2,%r0		# get IS,OS,US,QS
	llsw2	&1,%r0			# << 1 for inexact sticky
	extzv	&PS,&1,%r2,%r1		# get PS
	addw2	%r1,%r0			# add to the others

	lrsw3	&1,0(%ap),%r1		# align IS,OS,US,QS
	insv	%r1,&QS,&4,%r2		# put in IS,OS,US,QS
	insv	0(%ap),&PS,&1,%r2	# put in PS
	movw	%r2,_asr		# write _asr back
	RET

	.def	fpsetsticky;	.val	.;	.scl	-1;	.endef

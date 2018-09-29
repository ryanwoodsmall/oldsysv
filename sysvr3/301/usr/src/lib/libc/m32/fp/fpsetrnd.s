#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fpsetrnd.s"
.ident	"@(#)libc-m32:fp/fpsetrnd.s	1.5.1.2"
	.text
#----------------------------------
	.globl	_fp_hw		# to check hardware presence
	.globl	_asr		# simulated arithmetic status register
	.set	CSC_BIT,0x20000	# context switch control bit in ASR
				# is manually set after a "mmovta"
				# so that the new ASR is not lost on switch
#----------------------------------
	.align	4
	.def	fpsetround;	.val	fpsetround;	.scl	2;	.type	052;	.endef
	.globl	fpsetround
#----------------------------------
fpsetround:
	MCOUNT
	TSTW	_fp_hw
	je	.soft
#----------------------------------
# MAU
	save	&0
	addw2	&4,%sp
	mmovfa	0(%fp)			# read ASR
	extzv	&22,&2,0(%fp),%r0	# read old rounding mode
	insv	0(%ap),&22,&2,0(%fp)	# put new rounding mode
	orw2	&CSC_BIT,0(%fp)		# make sure this ASR is saved
	mmovta	0(%fp)			# write ASR back
	ret	&0
#----------------------------------
	.align	4
.soft:					# running software
	extzv	&22,&2,_asr,%r0		# extract rounding mode
	insv	0(%ap),&22,&2,_asr	# put new rounding mode
	RET

	.def	fpsetround;	.val	.;	.scl	-1;	.endef

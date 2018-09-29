#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fpgetrnd.s"
.ident	"@(#)libc-m32:fp/fpgetrnd.s	1.6"
	.text
#----------------------------------
	.globl	_fp_hw		# to check hardware present
	.globl	_asr		# simulated arithmetic status register
	.globl	_fpstart	# to force floating point startup
#----------------------------------
	.align	4
	.def	fpgetround;	.val	fpgetround;	.scl	2;	.type	052;	.endef
	.globl	fpgetround
fpgetround:
	MCOUNT
	TSTW	_fp_hw
	je	.soft
#----------------------------------
# MAU
	save	&0
	addw2	&4,%sp
	mmovfa	0(%fp)
	extzv	&22,&2,0(%fp),%r0
	ret	&0
#----------------------------------
.soft:					# running software
	extzv	&22,&2,_asr,%r0
	RET
	.def	fpgetround;	.val	.;	.scl	-1;	.endef
##############################################################################


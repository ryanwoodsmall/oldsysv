#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fpstart0.s"
.ident	"@(#)libc-m32:fp/fpstart0.s	1.4"
#
# _FPSTART -- null routine for floating-point startup.
#
# called only from __fpstart(), this module's purposes are:
# 1)to resolve the unsatisfied external reference to _fpstart(),
# for programs which do not do any floating-point math.
# 2) to define the global floating-point variables

	.globl	_fpstart
_fpstart:
	RET

# of the following variables, only _fp_hw depends on being == 0.
	.comm	_fp_hw,4	# .bss space for "int _fp_hw"
	.comm	_asr,4		# "int _asr"
	.comm	_fpftype,4	# "fp_ftype _fptype" == "int _fpftype"
	.comm	_fpfault,4	# "struct fp_fault * _fpfault"

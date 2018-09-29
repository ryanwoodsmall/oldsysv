#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- execle
.ident	"@(#)libc-m32:sys/execle.s	1.3"

# execle(file, arg1, arg2, ... , env);

	.globl	execle
	.globl	execve

execle:
	MCOUNT
	PUSHW	0(%ap)		#  file
	PUSHAW	4(%ap)		#  argv
	PUSHW	-20(%sp)	#  env
	CALL	-12(%sp),execve
	RET

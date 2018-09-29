#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- execl
.ident	"@(#)libc-m32:sys/execl.s	1.3"

# execl(file, arg1, arg2, ... , 0);

	.globl	execl
	.globl	execv

execl:
	MCOUNT
	PUSHW	0(%ap)
	PUSHAW	4(%ap)
	CALL	-8(%sp),execv
	RET

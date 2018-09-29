#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- execv
.ident	"@(#)libc-m32:sys/execv.s	1.7"

# execv(file, argv);

# where argv is a vector argv[0] ... argv[x], 0
# last vector element must be 0

	.globl	execv
	.globl	execve

execv:
	MCOUNT
	PUSHW	0(%ap)		#  file
	PUSHW	4(%ap)		#  argv
	PUSHW   environ         #  default environ
	CALL	-12(%sp),execve
	RET

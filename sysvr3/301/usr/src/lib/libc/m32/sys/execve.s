#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- execve
.ident	"@(#)libc-m32:sys/execve.s	1.3"

# execve(file, argv, env);

# where argv is a vector argv[0] ... argv[x], 0
# last vector element must be 0

	.set	_exece,59*8

	.globl	execve
	.globl	_cerror

execve:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_exece,%r1
	GATE
	JMP 	_cerror

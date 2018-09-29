#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
.ident	"@(#)libc-m32:sys/fork.s	1.3"
# C library -- fork

# pid = fork();

# r1 == 0 in parent process, r1 = 1 in child process.
# r0 == pid of child in parent, r0 == pid of parent in child.

	.set	_fork,2*8

	.globl	fork
	.globl	_cerror

fork:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_fork,%r1
	GATE
	jgeu	forkok
	JMP	_cerror
forkok:
	TSTW	%r1
	je	parent
	CLRW	%r0
parent:
	RET

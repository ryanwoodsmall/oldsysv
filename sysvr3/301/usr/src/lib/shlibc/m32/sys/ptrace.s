#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# ptrace -- C library
.ident	"@(#)libc-m32:sys/ptrace.s	1.3"

#	result = ptrace(req, pid, addr, data);

	.set	_ptrace,26*8

	.globl	ptrace
	.globl	_cerror
	.globl  errno

ptrace:
	MCOUNT
	CLRW	errno
	MOVW	&4,%r0
	MOVW	&_ptrace,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

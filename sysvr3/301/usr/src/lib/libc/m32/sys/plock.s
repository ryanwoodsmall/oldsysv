#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- plock
.ident	"@(#)libc-m32:sys/plock.s	1.3"

# error = plock(op)

	.set	_plock,45*8

	.globl	plock
	.globl	_cerror

plock:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_plock,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

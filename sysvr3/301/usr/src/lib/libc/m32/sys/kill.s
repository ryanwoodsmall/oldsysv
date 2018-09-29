#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- kill
.ident	"@(#)libc-m32:sys/kill.s	1.3"

	.set	_kill,37*8

	.globl	kill
	.globl	_cerror

kill:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_kill,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	CLRW	%r0
	RET

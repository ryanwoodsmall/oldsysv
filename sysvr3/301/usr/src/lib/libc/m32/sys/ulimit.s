#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- ulimit
.ident	"@(#)libc-m32:sys/ulimit.s	1.3"

	.set	_ulimit,63*8

	.globl	ulimit
	.globl	_cerror

ulimit:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_ulimit,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

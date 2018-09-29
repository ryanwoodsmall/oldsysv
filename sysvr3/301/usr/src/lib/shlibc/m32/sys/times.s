#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- times
.ident	"@(#)libc-m32:sys/times.s	1.3"

	.set	_times,43*8

	.globl	times
	.globl	_cerror

times:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_times,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

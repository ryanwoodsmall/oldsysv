#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/poll.s	1.1"

# C library -- poll

	.set	_poll,87*8

	.globl	poll
	.globl	_cerror

poll:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_poll,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

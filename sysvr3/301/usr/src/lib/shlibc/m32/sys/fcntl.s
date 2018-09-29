#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- fcntl
.ident	"@(#)libc-m32:sys/fcntl.s	1.3"

	.set	_fcntl,62*8

	.globl	fcntl
	.globl	_cerror

fcntl:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_fcntl,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

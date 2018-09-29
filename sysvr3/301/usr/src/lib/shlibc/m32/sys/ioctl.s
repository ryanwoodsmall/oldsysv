#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- ioctl
.ident	"@(#)libc-m32:sys/ioctl.s	1.3"

	.set	_ioctl,54*8

	.globl	ioctl
	.globl	_cerror

ioctl:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_ioctl,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

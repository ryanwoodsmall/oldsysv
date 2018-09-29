#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- mount
.ident	"@(#)libc-m32:sys/mount.s	1.3"

# error = mount(dev, file, flag)

	.set	_mount,21*8

	.globl	mount
	.globl  _cerror

mount:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_mount,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

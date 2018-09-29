#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- mknod
.ident	"@(#)libc-m32:sys/mknod.s	1.3"

# error = mknod(string, mode, major.minor);

	.set	_mknod,14*8

	.globl	mknod
	.globl  _cerror

mknod:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_mknod,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

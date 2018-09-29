#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- umount
.ident	"@(#)libc-m32:sys/umount.s	1.3"

	.set	_umount,22*8

	.globl	umount
	.globl	_cerror

umount:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_umount,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

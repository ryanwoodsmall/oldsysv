#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:sys/sysfs.s	1.2"
# C library -- sysfs

	.set	_sysfs,84*8

	.globl	sysfs
	.globl	_cerror


sysfs:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_sysfs,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

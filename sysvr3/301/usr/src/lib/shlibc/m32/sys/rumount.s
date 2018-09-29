#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:sys/rumount.s	1.3"

	.set	_rumount,73*8

	.globl	rumount
	.globl	_cerror

rumount:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_rumount,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

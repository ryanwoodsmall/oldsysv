#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:sys/rdump.s	1.2"

	.set	_rdump ,69*8

	.globl	rdump
	.globl	_cerror

rdump:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_rdump,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- dup
.ident	"@(#)libc-m32:sys/dup.s	1.3"

#	f = dup(of [ ,nf])
#	f == -1 for error

	.set	_dup,41*8

	.globl	dup
	.globl	_cerror

dup:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_dup,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- open
.ident	"@(#)libc-m32:sys/open.s	1.3"

# file = open(string, mode)

# file == -1 means error

	.set	_open,5*8

	.globl	open
	.globl  _cerror

open:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_open,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

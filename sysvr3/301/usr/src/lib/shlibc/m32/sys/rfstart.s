#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:sys/rfstart.s	1.2"

	.set	_rfstart,74*8

	.globl	rfstart
	.globl	_cerror

rfstart:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_rfstart,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

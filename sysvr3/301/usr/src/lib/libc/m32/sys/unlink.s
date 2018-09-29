#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- unlink
.ident	"@(#)libc-m32:sys/unlink.s	1.3"

# error = unlink(string);

	.set	_unlink,10*8

	.globl	unlink
	.globl  _cerror

unlink:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_unlink,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

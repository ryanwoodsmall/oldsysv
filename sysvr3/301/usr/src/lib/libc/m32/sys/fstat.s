#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- fstat
.ident	"@(#)libc-m32:sys/fstat.s	1.3"

# error = fstat(file, statbuf);

# char statbuf[34]

	.set	_fstat,28*8

	.globl	fstat
	.globl	_cerror

fstat:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_fstat,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

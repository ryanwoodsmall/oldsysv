#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# pipe -- C library
.ident	"@(#)libc-m32:sys/pipe.s	1.3"

#	pipe(f)
#	int f[2];

	.set	_pipe,42*8

	.globl	pipe
	.globl  _cerror

pipe:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_pipe,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	MOVW	%r0,*0(%ap)
	MOVW	0(%ap),%r0
	MOVW	%r1,4(%r0)
	CLRW	%r0
	RET

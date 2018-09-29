#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- chmod
.ident	"@(#)libc-m32:sys/chmod.s	1.3"

# error = chmod(string, mode);

	.set	_chmod,15*8

	.globl	chmod
	.globl	_cerror

chmod:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_chmod,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

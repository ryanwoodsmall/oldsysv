#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- link
.ident	"@(#)libc-m32:sys/link.s	1.3"

# error = link(old-file, new-file);

	.set	_link,9*8

	.globl	link
	.globl	_cerror

link:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_link,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

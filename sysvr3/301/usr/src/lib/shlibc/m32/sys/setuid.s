#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- setuid
.ident	"@(#)libc-m32:sys/setuid.s	1.3"

# error = setuid(uid);

	.set	_setuid,23*8

	.globl	setuid
	.globl  _cerror

setuid:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_setuid,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

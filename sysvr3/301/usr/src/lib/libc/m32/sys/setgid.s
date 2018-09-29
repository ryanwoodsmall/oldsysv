#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- setgid
.ident	"@(#)libc-m32:sys/setgid.s	1.3"

# error = setgid(uid);

	.set	_setgid,46*8

	.globl	setgid
	.globl  _cerror

setgid:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_setgid,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

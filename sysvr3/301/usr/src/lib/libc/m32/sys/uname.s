#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- uname
.ident	"@(#)libc-m32:sys/uname.s	1.3"

	.set	_utssys,57*8
	.set	_uname,0

	.globl	uname
	.globl	_cerror

uname:
	MCOUNT
	MCOUNT
	PUSHW	0(%ap)
	PUSHW	&0
	PUSHW	&_uname
	CALL	-12(%sp),sys
	RET

sys:
	MOVW	&4,%r0
	MOVW	&_utssys,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	CLRW	%r0
	RET

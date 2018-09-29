#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- acct
.ident	"@(#)libc-m32:sys/acct.s	1.3"

	.set	_acct,51*8

	.globl	acct
	.globl  _cerror

acct:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_acct,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/putmsg.s	1.1"

# C library -- putmsg

	.set	_putmsg,86*8

	.globl	putmsg
	.globl	_cerror

putmsg:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_putmsg,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

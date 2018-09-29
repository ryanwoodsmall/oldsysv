#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/getmsg.s	1.1"

# C library -- getmsg

	.set	_getmsg,85*8

	.globl	getmsg
	.globl	_cerror

getmsg:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_getmsg,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

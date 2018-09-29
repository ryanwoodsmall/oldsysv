#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# getpid -- get process ID
.ident	"@(#)libc-m32:sys/getpid.s	1.3"

	.set	_getpid,20*8

	.globl	getpid

getpid:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_getpid,%r1
	GATE
	RET

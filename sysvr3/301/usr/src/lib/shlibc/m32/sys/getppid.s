#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# getppid -- get parent process ID
.ident	"@(#)libc-m32:sys/getppid.s	1.3"

	.set	_getpid,20*8

	.globl	getppid

getppid:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_getpid,%r1
	GATE
	MOVW	%r1,%r0
	RET

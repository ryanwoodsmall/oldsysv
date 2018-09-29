#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- getgid
.ident	"@(#)libc-m32:sys/getgid.s	1.3"

# gid = getgid();
# returns real gid

	.set	_getgid,47*8

	.globl	getgid

getgid:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_getgid,%r1
	GATE
	RET

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- getuid
.ident	"@(#)libc-m32:sys/getuid.s	1.3"

# uid = getuid();
#  returns real uid

	.set	_getuid,24*8

	.globl	getuid

getuid:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_getuid,%r1
	GATE
	RET

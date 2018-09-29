#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#  C library -- sync
.ident	"@(#)libc-m32:sys/sync.s	1.3"

	.set	_sync,36*8

	.globl	sync

sync:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_sync,%r1
	GATE
	RET

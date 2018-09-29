#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- _exit
.ident	"@(#)libc-m32:sys/exit.s	1.3"

# _exit(code)
# code is return in r0 to system
# Same as plain exit, for user who want to define their own exit.

	.set	exit,1*8

	.globl	_exit

_exit:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&exit,%r1
	GATE

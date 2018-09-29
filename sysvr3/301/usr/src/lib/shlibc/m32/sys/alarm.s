#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library - alarm
.ident	"@(#)libc-m32:sys/alarm.s	1.3"

	.set	_alarm,27*8

	.globl	alarm

	.align	1
alarm:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_alarm,%r1
	GATE
	RET

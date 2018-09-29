#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library - pause
.ident	"@(#)libc-m32:sys/pause.s	1.3"

	.set	_pause,29*8

	.globl	pause
	.globl	_cerror

pause:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_pause,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET

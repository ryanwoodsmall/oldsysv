#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#  C - library stime
.ident	"@(#)libc-m32:sys/stime.s	1.3"

	.set	_stime,25*8

	.globl	stime
	.globl  _cerror

stime:
	MCOUNT
	MOVW	*0(%ap),0(%ap)	# copy time to set
	MOVW	&4,%r0
	MOVW	&_stime,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

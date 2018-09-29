#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


.ident	"@(#)libc-m32:sys/sys3b.s	1.3"
# C library -- sys3b

# file = sys3b(cmd,arg1[,arg2])

# file == -1 means error

	.set	_sys3b,50*8

	.globl	sys3b
	.globl  _cerror

sys3b:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_sys3b,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

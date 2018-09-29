#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- creat
.ident	"@(#)libc-m32:sys/creat.s	1.3"

# file = creat(string, mode);
# file == -1 if error

	.set	_creat,8*8

	.globl	creat
	.globl	_cerror

creat:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_creat,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

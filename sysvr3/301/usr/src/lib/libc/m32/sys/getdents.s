#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- getdents
	.ident	"@(#)libc-m32:sys/getdents.s	1.2.1.1"
	.file 	"getdents.s"

# num = getdents(file, buffer, count, flag);
# num is number of bytes read; num == -1 means error

	.set	_getdents,81*8

	.globl	getdents
	.globl  _cerror

getdents:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_getdents,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

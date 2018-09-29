#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- mkdir
	.ident	"@(#)libc-m32:sys/mkdir.s	1.3"
	.file	"mkdir.s"

# file = mkdir(string, mode);
# file == -1 if error

	.set	_mkdir,80*8

	.globl	mkdir
	.globl	_cerror

mkdir:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_mkdir,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

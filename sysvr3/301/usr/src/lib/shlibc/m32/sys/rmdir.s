#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- rmdir
	.ident	"@(#)libc-m32:sys/rmdir.s	1.4"
	.file	"rmdir.s"

	.set	_rmdir,79*8

	.globl	rmdir
	.globl	_cerror

rmdir:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_rmdir,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

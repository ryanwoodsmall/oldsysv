#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- lseek
.ident	"@(#)libc-m32:sys/lseek.s	1.3"

# error = lseek(file, offset, ptr);


	.set	_lseek,19*8

	.globl	lseek
	.globl  _cerror

lseek:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_lseek,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

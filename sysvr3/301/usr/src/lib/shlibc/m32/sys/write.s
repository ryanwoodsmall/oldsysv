#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- write
.ident	"@(#)libc-m32:sys/write.s	1.3"

# nwritten = write(file, buffer, count);
# nwritten == -1 means error

	.set	_write,4*8

	.globl	write
	.globl  _cerror

write:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_write,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

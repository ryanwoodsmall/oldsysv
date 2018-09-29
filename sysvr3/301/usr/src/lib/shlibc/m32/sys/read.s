#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- read
.ident	"@(#)libc-m32:sys/read.s	1.3"

# nread = read(file, buffer, count);
# nread ==0 means eof; nread == -1 means error

	.set	_read,3*8

	.globl	read
	.globl  _cerror

read:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_read,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET

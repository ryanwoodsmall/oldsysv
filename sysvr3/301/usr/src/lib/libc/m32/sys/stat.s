#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- stat
.ident	"@(#)libc-m32:sys/stat.s	1.3"

# error = stat(string, statbuf);
# char statbuf[36]

	.set	_stat,18*8

	.globl	stat
	.globl  _cerror

stat:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_stat,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET

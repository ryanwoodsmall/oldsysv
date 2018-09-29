#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- chown
.ident	"@(#)libc-m32:sys/chown.s	1.3"

# error = chown(string, owner);

	.set	_chown,16*8

	.globl	chown
	.globl	_cerror

chown:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_chown,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET
